#include "work_package.h"

#include "fixed_area.h"
#include "priv_impl.h"

namespace elfin {

/* private */
struct WorkPackage::PImpl : public PImplBase<WorkPackage> {
    /* types */
    typedef std::unordered_set<std::string> NameSet;
    typedef std::unordered_map<std::string, NameSet> AdjacentNames;

    /* data */
    FixedAreaMap const& fixed_areas_;
    WorkAreas work_areas_;

    /* ctors */
    PImpl(WorkPackage& owner,
          FixedAreaMap const& fixed_areas_) :
        PImplBase(owner),
        fixed_areas_(fixed_areas_) {}

    /* accessors */
    SolutionMap make_solution_map() const {
        SolutionMap res;

        JUtil.error("Need fix: select best network variant.\n");

        // Simple case...
        for (auto const& [wa_name, wa] : work_areas_) {
            res.emplace(wa_name, wa->make_solution_minheap());
        }

        return res;
    }

    static void analyse_bp_network(std::string const& first_bp_name,
                                   JSON const& pg_network,
                                   AdjacentNames& adj_bps,
                                   AdjacentNames& adj_leaves)
    {
        // Parse the branchpoint network (collapse all non-branchpoint
        // joints).
        std::deque<std::string> bps_to_visit = {first_bp_name};
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

        while (not bps_to_visit.empty()) {
            auto const& bp_name = bps_to_visit.front();
            auto const& bp_nbs = pg_network[bp_name].at("neighbors");

            DEBUG_NOMSG(bp_nbs.size() <= 2);

            for (auto const& nb_name : bp_nbs) {
                // Collapse i.e. keep advancing name "pointer" until either
                // leaf or another branchpoint is encountered.

                if (visited.find(nb_name) == end(visited)) {
                    std::string joint_name = nb_name;
                    std::string last_name = bp_name;

                    JSON const* joint_nbs = &pg_network[joint_name].at("neighbors");
                    while (joint_nbs->size() == 2) {
                        // Not a branchpoint nor a leaf.
                        visited.insert(joint_name);

                        auto const nbs_itr = begin(*joint_nbs);

                        if (*nbs_itr != last_name) {
                            last_name = joint_name;
                            joint_name = *nbs_itr;
                        }
                        else {
                            last_name = joint_name;
                            joint_name = *(1 + nbs_itr);
                        }
                        joint_nbs = &pg_network[joint_name].at("neighbors");
                    }

                    auto const n_nbs = joint_nbs->size();
                    if (n_nbs == 1) {
                        sign_name(adj_leaves, bp_name, joint_name);
                    }
                    else {  // Branchpoint joint.
                        if (joint_name == bp_name) {
                            throw Unsupported("Branchpoint with circular connection "
                                              "is not yet supported. Violating branchpoint: " +
                                              bp_name + "\n");
                        }

                        sign_name(adj_bps, bp_name, joint_name);
                        sign_name(adj_bps, joint_name, bp_name);
                        bps_to_visit.push_back(joint_name);
                    }
                }
            }

            visited.insert(bp_name);
            bps_to_visit.pop_front();
        }
    }

    /* modifiers */
    void decimate(JSON const& pg_network,
                  NameSet& accumulator,
                  JSON& decimated_jsons,
                  size_t& dec_id)
    {
        if (not accumulator.empty()) {
            JSON decimated_json;
            // Trim accumulator joints by removing neighbor names that aren't
            // in this set.
            for (auto const& joint_name : accumulator) {
                decimated_json[joint_name] = pg_network[joint_name];  // Make mutable copy.

                auto& nbs = decimated_json[joint_name].at("neighbors");
                nbs.erase(std::remove_if(begin(nbs), end(nbs),
                [&accumulator](auto const & nb_name) {
                    return accumulator.find(nb_name) == end(accumulator);
                }),
                end(nbs));
            }

            auto const& dec_name = "dec." + std::to_string(dec_id++);
            decimated_jsons[dec_name] = decimated_json;
            accumulator.clear();
        }
    }

    void simple_decimate(JSON const& pg_network)
    {
        JSON decimated_jsons;
        NameSet accumulator;
        size_t dec_id = 0;

        auto const call_decimate = [&]() {
            decimate(pg_network,
                     accumulator,
                     decimated_jsons,
                     dec_id);
        };

        // Decimate the pg_network into a bunch of segments. A segment start
        // and end at either a leaf joint or a hinge (occupied) joint.
        for (auto const& [joint_name, joint_json] : pg_network.items()) {
            accumulator.insert(joint_name);

            if (joint_json.at("occupant") != "" and
                    joint_json.at("neighbors").size() > 1) {
                // This is a non-leaf joint that is occupied, therefore we
                // need to break the pg_network.
                call_decimate();

                // Re-insert current joint, which will be the beginning of the
                // next decimated segment.
                accumulator.insert(joint_name);
            }
        }

        // In case accumulator is not empty, do a last decimate to catch the
        // dangling pg_network segment.
        call_decimate();

        for (auto const& [dec_name, dec_json] : decimated_jsons.items()) {
            work_areas_.emplace(
                dec_name,
                std::make_unique<WorkArea>(dec_name, dec_json, fixed_areas_));

            PANIC_IF(work_areas_[dec_name]->joints.empty(),
                     ShouldNotReach("PathGuide network \"" + _.name +
                                    "\" has no joints associated. "
                                    "Error in parsing or input spec, maybe?"));
        }
    }

    void star_decimate(JSON const& pg_network,
                       AdjacentNames const& adj_leaves)
    {
        // - 1. Find compatible hubs.
        // - 2. Define 1H work area for each star arm.
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

    void parse(JSON const & pg_network)
    {
        // Check whether we need to use advanced rules to break up the
        // pg_network.
        for (auto const& [joint_name, joint_json] : pg_network.items()) {
            if (joint_json.at("neighbors").size() > 2) {
                // Found a branchpoint. There are two cases for a complex
                // pg_network:
                //  - The simple "star" case - 1-hub network in which
                //    except for the hub, all arms are 1h.
                //
                //  - The complex type, in which multiple hubs are present and
                //    connect to each other.
                AdjacentNames adj_bps, adj_leaves;
                analyse_bp_network(joint_name, pg_network, adj_bps, adj_leaves);

                for (auto const& [bp_name, bp_nbs] : adj_bps) {
                    std::ostringstream oss;
                    oss << bp_name << " connects to bps:\n";
                    for (auto const& nb_name : bp_nbs)
                        oss << nb_name << "\n";
                    JUtil.warn(oss.str().c_str());
                }

                // for (auto const& [bp_name, leaf_nbs] : adj_leaves) {
                //     std::ostringstream oss;
                //     oss << bp_name << " connects to leaves:\n";
                //     for (auto const& leaf_name : leaf_nbs)
                //         oss << leaf_name << "\n";
                //     JUtil.warn(oss.str().c_str());
                // }

                if (adj_bps.size() == 0) {
                    star_decimate(pg_network, adj_leaves);
                }
                else {
                    allorders_decimate(pg_network, adj_bps, adj_leaves);
                }

                // There can only be one single hub network in a pg_network
                // since all UIJoints are connected.
                return;
            }
        }

        simple_decimate(pg_network);
    }
};

/* public */
/* ctors */
WorkPackage::WorkPackage(std::string const& pg_nw_name,
                         JSON const& pg_network,
                         FixedAreaMap const& fixed_areas_) :
    pimpl_(new_pimpl<PImpl>(*this, fixed_areas_)),
    name(pg_nw_name)
{
    pimpl_->parse(pg_network);
}

/* dtors */
WorkPackage::~WorkPackage() {}

/* accessors */
WorkAreas const& WorkPackage::work_areas() const {
    return pimpl_->work_areas_;
}

/* accessors */
SolutionMap WorkPackage::make_solution_map() const {
    return pimpl_->make_solution_map();
}

/* modifiers */
void WorkPackage::solve() {
    JUtil.error("Need fix: solving complex networks.\n");

    for (auto& [wa_name, wa] : pimpl_->work_areas_) {
        wa->solve();
    }
}

}  /* elfin */