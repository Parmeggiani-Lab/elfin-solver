#include "work_package.h"

#include "input_manager.h"
#include "fixed_area.h"
#include "priv_impl.h"

namespace elfin {

/* private */
struct WorkPackage::PImpl : public PImplBase<WorkPackage> {
    using PImplBase::PImplBase;

    /* types */
    typedef std::vector<std::string> Names;
    typedef std::unordered_set<std::string> NameSet;
    typedef std::unordered_map<std::string, NameSet> AdjacentNames;

    /* data */
    size_t dec_id_ = 0;
    WorkAreas work_areas_;

    UIModuleMap fake_occupants_;

    /* accessors */
    SolutionMap make_solution_map() const {
        SolutionMap res;

        auto const add_heaps = [&](WorkAreas const & verse) {
            for (auto const& wa : verse)
                res.emplace(wa->name, wa->make_solution_minheap());
        };

        add_heaps(work_areas_);

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

        NameSet const seg_name_set(begin(seg_names), end(seg_names));

        // Trim seg_names joints by removing neighbor names that aren't
        // in this set.
        for (auto const& joint_name : seg_names) {
            decimated_json[joint_name] = pg_network.at(joint_name);  // Make mutable copy.

            auto& nbs = decimated_json[joint_name].at("neighbors");

            nbs.erase(std::remove_if(begin(nbs), end(nbs),
            [&seg_name_set](auto const & nb_name) {
                return seg_name_set.find(nb_name) == end(seg_name_set);
            }),
            end(nbs));
        }

        std::string const& dec_name = "dec." + std::to_string(dec_id_++);

        work_areas_.emplace_back(
            std::make_unique<WorkArea>(dec_name, decimated_json, fixed_areas));
    }

    void analyse_pg_network(std::string const& leaf_name,
                            JSON const& pg_network,
                            FixedAreaMap const& fixed_areas)
    {
        // Verify that it's a leaf.
        DEBUG_NOMSG(pg_network.at(leaf_name).at("neighbors").size() != 1);

        auto const is_bp = [&pg_network](std::string const & name) {
            return pg_network.at(name).at("neighbors").size() > 2;
        };
        auto const not_occupied = [&pg_network](std::string const & name) {
            return pg_network.at(name).at("occupant") == "";
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
                    // sign_name(adj_leaves, start_name, joint_name);
                }
                else {  // Branchpoint.
                    if (joint_name == start_name) {
                        throw Unsupported("Branchpoint with circular connection "
                                          "is not yet supported. Violating branchpoint: " +
                                          start_name + "\n");
                    }

                    frontier.push_back(joint_name);

                    // sign_name(adj_bps, start_name, joint_name);
                    // sign_name(adj_bps, joint_name, start_name);
                }

                decimate(seg_names, pg_network, fixed_areas);
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
        for (auto& wa : work_areas_)
            wa->solve();
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
size_t WorkPackage::n_work_areas() const {
    return pimpl_->work_areas_.size();
}

WorkAreas const& WorkPackage::work_areas() const {
    return pimpl_->work_areas_;
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