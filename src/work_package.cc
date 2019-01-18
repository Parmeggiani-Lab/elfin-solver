#include "work_package.h"

#include "input_manager.h"
#include "fixed_area.h"
#include "priv_impl.h"
#include "move_heap.h"

namespace elfin {

/* private */
struct WorkPackage::PImpl : public PImplBase<WorkPackage> {
    using PImplBase::PImplBase;

    /* types */
    typedef std::vector<std::string> Names;
    typedef std::unordered_set<std::string> NameSet;
    typedef SPMap<WorkArea> WorkAreaMap;

    typedef WorkArea* WaKey;
    struct WaLink;
    typedef WaLink* WaLinkKey;
    typedef std::unique_ptr<WaLink> WaLinkSP;
    struct WaLink {
        WaKey key;
        float sum_pg_dist;
        WaLink(WaKey _key, float _dist) : key(_key), sum_pg_dist(_dist) {}
        struct PtrGreater {
            bool operator()(WaLinkKey const lhs,
                            WaLinkKey const rhs) {
                return lhs->sum_pg_dist > rhs->sum_pg_dist;
            }
        };
    };
    typedef std::vector<WaLinkSP> WaLinks;
    typedef std::vector<WaLinkKey> WaLinkKeys;
    typedef std::unordered_map<std::string, WaLinkKeys> CompactGraph;
    typedef MoveHeap<WaLinkKey,
            WaLinkKeys,
            WaLink::PtrGreater> WaLinkHeap;

    /* data */
    size_t dec_id_ = 0;

    WorkAreaMap wa_map_;
    WorkAreaKeys wa_keys_;
    WaLinks wa_links_;

    WaLinkHeap wl_heap_;
    CompactGraph graph_;
    UIModuleMap new_occupants_;

    /* accessors */
    SolutionMap make_solution_map() const {
        SolutionMap res;

        for (auto const& [wa_name, wa] : wa_map_)
            res.emplace(wa_name, wa->make_solution_minheap());

        return res;
    }

    /* modifiers */
    void parse(FixedAreaMap const& fixed_areas,
               JSON const& pg_network)
    {
        // Find leaf node to begin traversal analysis with.
        for (auto const& [joint_name, joint_json] : pg_network.items()) {
            auto const n_nbs = joint_json.at("neighbors").size();

            if (n_nbs == 1) {
                analyse_pg_network(joint_name, pg_network, fixed_areas);
                return;
            }
            else if (n_nbs == 0) {
                throw BadSpec("Joint " + joint_name + " has no neighbours.");
            }
        }

        throw BadSpec("PathGuide network " + _.name +
                      " has no leaf joint to begin traversal with.");
    }

    void decimate(Names const& seg_names,
                  JSON const& pg_network,
                  FixedAreaMap const& fixed_areas)
    {
        if (seg_names.empty()) return;

        JSON decimated_json;
        float sum_pg_dist = 0.0f;

        NameSet const seg_name_set(begin(seg_names), end(seg_names));

        for (auto itr = seg_names.begin(); itr != seg_names.end(); ++itr) {
            auto const& joint_name = *itr;

            // Trim seg_names joints by removing neighbor names that aren't
            // in this set.
            {
                decimated_json[joint_name] = pg_network.at(joint_name);  // Make mutable copy.

                auto& nbs = decimated_json.at(joint_name).at("neighbors");

                nbs.erase(std::remove_if(begin(nbs), end(nbs),
                [&seg_name_set](auto const & nb_name) {
                    return seg_name_set.find(nb_name) == end(seg_name_set);
                }),
                end(nbs));
            }

            // Accumulate joint dists.
            {
                auto const next_itr = (1 + itr);
                if (next_itr != seg_names.end()) {
                    auto const& next_name = *next_itr;

                    // Cheat by interpretting joint json as tx_json.
                    auto const& curr_pos = Transform(pg_network.at(joint_name)).collapsed();
                    auto const& next_pos = Transform(pg_network.at(next_name)).collapsed();

                    sum_pg_dist += next_pos.dist_to(curr_pos);
                }
            }
        }


        {
            // WorkArea.
            std::string const& dec_name = "dec." + std::to_string(dec_id_++);
            auto new_wa = std::make_unique<WorkArea>(
                              dec_name, decimated_json, fixed_areas);
            auto wa_key = new_wa.get();

            wa_map_.emplace(dec_name, std::move(new_wa));
            wa_keys_.push_back(wa_key);

            // WaLink.
            auto new_wl = std::make_unique<WaLink>(wa_key, sum_pg_dist);
            auto const wl_key = new_wl.get();

            wa_links_.emplace_back(std::move(new_wl));
            wl_heap_.push(wl_key);

            // Graph.
            auto const& src_name = seg_names.front();
            auto const& dst_name = seg_names.back();

            if (graph_.find(src_name) == end(graph_))
                graph_[src_name] = WaLinkKeys();
            graph_[src_name].push_back(wl_key);

            if (graph_.find(dst_name) == end(graph_))
                graph_[dst_name] = WaLinkKeys();
            graph_[dst_name].push_back(wl_key);

            // Provisional occupants.
            auto const gen_occ_name = [](std::string const & name) {
                return name + ".provis";
            };

            auto const need_occupant = [&] (std::string const & name) {
                auto const& joint_json = pg_network.at(name);

                return new_occupants_.find(gen_occ_name(name)) == end(new_occupants_) and
                       joint_json.at("neighbors").size() > 2 and
                       joint_json.at("occupant") == "";
            };

            auto const provide_occupant = [&](std::string const & name) {
                auto const& joint_json = pg_network.at(name);
                auto const& occ_name =  gen_occ_name(name);

                JUtil.error("Need occupant at %s\n", name.c_str());

                auto const& pos = Transform(pg_network.at(name)).collapsed();

                auto new_occ_uimod = std::make_unique<UIModule>(occ_name, pos);

                // Fill provis_hubs.
                size_t const degree = joint_json.at("neighbors").size();
                for (auto const ptmod : XDB.hubs().items()) {
                    if (ptmod->counts().all_interfaces() >= degree)
                        new_occ_uimod->provis_hubs.push_back(ptmod);
                }

                auto const new_occ_ptr = new_occ_uimod.get();
                new_occupants_.emplace(occ_name, std::move(new_occ_uimod));

                auto& occupant = wa_key->joints.at(name)->occupant;
                occupant.parent_name = "provis";
                occupant.ui_module = new_occ_ptr;
            };

            bool const src_need_occupant = need_occupant(src_name);
            if (src_need_occupant)
                provide_occupant(src_name);

            bool const dst_need_occupant = need_occupant(dst_name);
            if (dst_need_occupant)
                provide_occupant(dst_name);

            // Determine new work area type.

        }
    }

    void analyse_pg_network(std::string const & leaf_name,
                            JSON const & pg_network,
                            FixedAreaMap const & fixed_areas)
    {
        // Verify that it's a leaf.
        DEBUG_NOMSG(pg_network.at(leaf_name).at("neighbors").size() != 1);

        std::deque<std::string> frontier = {leaf_name};
        NameSet visited;

        while (not frontier.empty()) {
            auto const& start_name = frontier.front();
            if (visited.find(start_name) == end(visited)) {
                visited.insert(start_name);

                auto const& start_nbs = pg_network.at(start_name).at("neighbors");

                for (auto const& nb_name : start_nbs) {
                    if (visited.find(nb_name) != end(visited)) continue;

                    Names seg_names = {start_name};
                    // Collapse i.e. keep advancing name "pointer" until either
                    // leaf or another branchpoint is encountered.

                    std::string joint_name = nb_name;
                    std::string last_name = start_name;

                    JSON const* joint_nbs = &pg_network.at(joint_name).at("neighbors");
                    while (joint_nbs->size() == 2) {
                        // Not a branchpoint nor a leaf.
                        visited.insert(joint_name);
                        seg_names.push_back(joint_name);

                        // Break pg_network at occupied joint.
                        if (pg_network.at(joint_name).at("occupant") != "") {
                            decimate(seg_names, pg_network, fixed_areas);

                            // Clear segment and re-insert current joint, which
                            // will be the beginning of the next decimated
                            // segment.
                            seg_names.clear();
                            seg_names.push_back(joint_name);
                        }

                        // Find next neighbor.
                        {
                            auto const nbs_itr = begin(*joint_nbs);

                            if (*nbs_itr != last_name) {
                                last_name = joint_name;
                                joint_name = *nbs_itr;
                            }
                            else {
                                last_name = joint_name;
                                joint_name = *(1 + nbs_itr);
                            }

                            joint_nbs = &pg_network.at(joint_name).at("neighbors");
                        }
                    }

                    // Upon exit of the while loop above, joint_name must be
                    // either a leaf or a branchpoint, which we need to
                    // include at the end of the current segment.
                    seg_names.push_back(joint_name);

                    auto const n_nbs = joint_nbs->size();
                    if (n_nbs == 1) {  // Leaf.
                    }
                    else {  // Branchpoint.
                        if (joint_name == start_name) {
                            throw Unsupported("Branchpoint with circular connection "
                                              "is not yet supported. Violating branchpoint: " +
                                              start_name + "\n");
                        }

                        frontier.push_back(joint_name);
                    }

                    decimate(seg_names, pg_network, fixed_areas);
                }
            }

            frontier.pop_front();
        }

        WaLinkHeap tmp;
        while (not wl_heap_.empty()) {
            auto wl = wl_heap_.top_and_pop();
            tmp.push(wl);
            auto const wa_key = wl->key;
            auto const& leaves = wa_key->leaf_joints;
            JUtil.error("%s to %s, dist: %.4f, wa: %p\n",
                        leaves.at(0)->name.c_str(), leaves.at(1)->name.c_str(), wl->sum_pg_dist, wa_key);
        }
        wl_heap_ = tmp;
    }

    void solve() {
        // Solve shortest distance first.
        WaLinkHeap tmp;
        while (not wl_heap_.empty()) {
            auto wl = wl_heap_.top_and_pop();
            wl->key->solve();
            tmp.push(wl);
        }
        wl_heap_ = tmp;
    }
};

/* public */
/* ctors */
WorkPackage::WorkPackage(std::string const& pg_nw_name,
                         JSON const& pg_network,
                         FixedAreaMap const& fixed_areas_) :
    pimpl_(new_pimpl<PImpl>(*this)),
    name(pg_nw_name)
{
    pimpl_->parse(fixed_areas_, pg_network);
}

/* dtors */
WorkPackage::~WorkPackage() {}

/* accessors */
size_t WorkPackage::n_work_area_keys() const {
    return pimpl_->wa_keys_.size();
}

WorkAreaKeys const& WorkPackage::work_area_keys() const {
    return pimpl_->wa_keys_;
}

/* accessors */
SolutionMap WorkPackage::make_solution_map() const {
    return pimpl_->make_solution_map();
}

/* modifiers */
void WorkPackage::solve() {
    pimpl_->solve();
}

}  /* elfin */