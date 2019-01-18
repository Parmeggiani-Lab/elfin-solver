#include "work_package.h"

#include "input_manager.h"
#include "fixed_area.h"
#include "priv_impl.h"

namespace elfin {

/* private */
struct WorkPackage::PImpl : public PImplBase<WorkPackage> {
    using PImplBase::PImplBase;

    /* types */
    typedef std::unordered_set<std::string> NameSet;
    typedef std::unordered_map<std::string, NameSet> AdjacentNames;
    typedef std::vector<WorkVerse> WorkVerses;

    /* data */
    WorkVerse det_verse_;
    WorkVerses ndet_verses_;

    UIModuleMap fake_occupants_;

    /* accessors */
    WorkVerse const& get_best_ndet_verse() const {
        DEBUG_NOMSG(ndet_verses_.empty());

        JUtil.error("FIXME: select best network variant.\n");
        return ndet_verses_.at(0);
    }

    SolutionMap make_solution_map() const {
        SolutionMap res;

        auto const add_heaps = [&](WorkVerse const & verse) {
            for (auto const& wa : verse)
                res.emplace(wa->name, wa->make_solution_minheap());
        };

        add_heaps(det_verse_);

        if (not ndet_verses_.empty()) {
            add_heaps(get_best_ndet_verse());
        }

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

    JSON decimate(JSON const& pg_network,
                  NameSet const& seg_names)
    {
        JSON decimated_json;

        // Trim seg_names joints by removing neighbor names that aren't
        // in this set.
        for (auto const& joint_name : seg_names) {
            decimated_json[joint_name] = pg_network.at(joint_name);  // Make mutable copy.

            auto& nbs = decimated_json[joint_name].at("neighbors");
            nbs.erase(std::remove_if(begin(nbs), end(nbs),
            [&seg_names](auto const & nb_name) {
                return seg_names.find(nb_name) == end(seg_names);
            }),
            end(nbs));
        }

        return decimated_json;
    }

    void analyse_pg_network(std::string const& leaf_name,
                            JSON const& pg_network,
                            FixedAreaMap const& fixed_areas)
    {
        // Verify that it's a leaf.
        DEBUG_NOMSG(pg_network.at(leaf_name).at("neighbors").size() != 1);

        size_t dec_id = 0;
        auto const create_det_wa = [&](NameSet const & seg_names) {
            if (seg_names.empty()) return;

            std::string const& dec_name = "dec." + std::to_string(dec_id++);
            auto const& dec_json = decimate(pg_network, seg_names);
            det_verse_.emplace_back(
                std::make_unique<WorkArea>(dec_name, dec_json, fixed_areas));
        };

        auto const is_bp = [&pg_network](std::string const & name) {
            return pg_network.at(name).at("neighbors").size() > 2;
        };
        auto const not_occupied = [&pg_network](std::string const & name) {
            return pg_network.at(name).at("occupant") == "";
        };
        auto const is_ndet = [&is_bp, &not_occupied](std::string const & name) {
            // Non-deterministic means a branchpoint without specified occupant.
            return is_bp(name) and not_occupied(name);
        };

        AdjacentNames adj_bps, adj_leaves;
        std::deque<std::string> frontier = {leaf_name};
        NameSet visited;

        auto const sign_name = [](AdjacentNames & adjacency,
                                  std::string const & owner,
                                  std::string const & new_name)
        {
            if (adjacency.find(owner) == end(adjacency)) {
                adjacency[owner] = NameSet();
            }
            adjacency[owner].insert(new_name);
        };

        while (not frontier.empty()) {
            auto const& start_name = frontier.front();
            visited.insert(start_name);

            auto const& start_nbs = pg_network.at(start_name).at("neighbors");

            for (auto const& nb_name : start_nbs) {
                if (visited.find(nb_name) != end(visited)) continue;

                NameSet seg_names = {start_name};
                // Collapse i.e. keep advancing name "pointer" until either
                // leaf or another branchpoint is encountered.

                std::string joint_name = nb_name;
                std::string last_name = start_name;

                JSON const* joint_nbs = &pg_network.at(joint_name).at("neighbors");
                while (joint_nbs->size() == 2) {
                    // Not a branchpoint nor a leaf.
                    visited.insert(joint_name);
                    seg_names.insert(joint_name);

                    // Break pg_network at occupied joint.
                    if (pg_network.at(joint_name).at("occupant") != "") {
                        create_det_wa(seg_names);

                        // Clear segment and re-insert current joint, which
                        // will be the beginning of the next decimated
                        // segment.
                        seg_names.clear();
                        seg_names.insert(joint_name);
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
                seg_names.insert(joint_name);

                auto const n_nbs = joint_nbs->size();
                if (n_nbs == 1) {  // Leaf.
                    if (is_ndet(start_name)) {
                        sign_name(adj_leaves, start_name, joint_name);
                    }
                    else {
                        create_det_wa(seg_names);
                    }
                }
                else {  // Branchpoint.
                    if (joint_name == start_name) {
                        throw Unsupported("Branchpoint with circular connection "
                                          "is not yet supported. Violating branchpoint: " +
                                          start_name + "\n");
                    }

                    frontier.push_back(joint_name);

                    if (is_ndet(start_name) or is_ndet(joint_name)) {
                        if (is_bp(start_name) and is_bp(joint_name)) {
                            sign_name(adj_bps, start_name, joint_name);
                            sign_name(adj_bps, joint_name, start_name);
                        }
                    }
                    else {
                        create_det_wa(seg_names);
                    }
                }
            }

            frontier.pop_front();
        }

        // Debug printing.
        for (auto const& [bp_name, bp_nbs] : adj_bps) {
            std::ostringstream oss;
            oss << bp_name << " connects to bps:\n";
            for (auto const& nb_name : bp_nbs)
                oss << nb_name << "\n";
            JUtil.warn(oss.str().c_str());
        }
    }

    void star_decimate(JSON const& pg_network,
                       AdjacentNames const& adj_leaves)
    {
        // Find compatible hubs.
        DEBUG_NOMSG(adj_leaves.size() != 1);
        auto const& [bp_name, leaves] = *begin(adj_leaves);

        auto const max_degree = XDB.max_bp_degree();
        PANIC_IF(leaves.size() > max_degree,
                 BadSpec("Branchpoint \"" + bp_name + "\" has a degree of " +
                         to_string(leaves.size()) + ", but max degree supported " +
                         "by modules in XDB is " + to_string(max_degree)));

        // Create fake hub UIModules for each compatible hub at the start
        // center branchpoint.


        // Create all evaluation verses. There's one verse variant per fake
        // center hub.

        // - 3. Solve for each starting 1H work area to find best score, then
        //   solve for remaining 1H arms.
        throw ShouldNotReach("Good!");
    }

    void allorders_decimate(JSON const& pg_network,
                            AdjacentNames const& adj_bps,
                            AdjacentNames const& adj_leaves)
    {
        // - 1. Find compatible hubs per bp.
        // - 2. Create (bp_name ^ team cksm), (bp_name ^ team cksm) cache map.
        // - 3. Create initial segments.
        //
        //   ... Run later during solve_all() ...
        // - 4. Traverse edges in all neighboring orders and solve segments
        //   on-the-fly if not found in cache.
        throw ShouldNotReach("Bad!");
    }

    void solve() {
        auto const solve_verse = [](WorkVerse & verse) {
            for (auto& wa : verse)
                wa->solve();
        };

        for (auto& verse : ndet_verses_)
            solve_verse(verse);

        solve_verse(det_verse_);
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
size_t WorkPackage::det_verse_size() const {
    return pimpl_->det_verse_.size();
}

WorkVerse const& WorkPackage::det_verse() const {
    return pimpl_->det_verse_;
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