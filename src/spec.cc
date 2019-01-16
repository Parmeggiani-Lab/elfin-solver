#include "spec.h"

#include "debug_utils.h"
#include "options.h"
#include "json.h"

namespace elfin {

/* private */
struct Spec::PImpl {
    /* types */
    typedef std::unordered_set<std::string> NameSet;
    typedef std::unordered_map<std::string, NameSet> AdjacentNames;

    /* data */
    Spec& _;

    /* ctors */
    PImpl(Spec& interface) : _(interface) { }

    /* accessors */
    static void analyse_bp_network(std::string const& first_bp_name,
                                   JSON const& pg_network,
                                   AdjacentNames& adj_bps,
                                   AdjacentNames& adj_leaves) {
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
    void parse(Options const & options) {
        _.work_areas_.clear();
        _.fixed_areas_.clear();

        auto const& spec_file = options.spec_file;
        JUtil.info("Parsing spec file: %s\n", spec_file.c_str());

        PANIC_IF(spec_file.empty(),
                 BadArgument("No input spec file provided.\n"));

        PANIC_IF(not JUtil.file_exists(spec_file.c_str()),
                 BadArgument("Input file \"" + spec_file + "\" does not exist.\n"));

        try {
            auto const& spec_json = parse_json(spec_file);
            auto const& networks_json = spec_json.at("networks");
            auto const& pg_networks_json = spec_json.at("pg_networks");

            JUtil.info("Input spec has %zu networks and %zu pg_networks.\n",
                       networks_json.size(),
                       pg_networks_json.size());

            // Initialize fixed areas first so work areas can refer to fixed
            // modules as occupants or hinges.
            for (auto& [name, json] : networks_json.items()) {
                digest_network(name, json);
            }

            for (auto& [name, json] : pg_networks_json.items()) {
                digest_pg_network(name, json);
            }
        } catch (JSON::exception const& je) {
            JSON_LOG_EXIT(je);
        }

        JUtil.info("Parsed %zu fixed areas and %zu work areas\n",
                   _.fixed_areas_.size(), _.work_areas_.size());
    }

    void digest_network(std::string const & name, JSON const & network) {
        _.fixed_areas_.emplace(
            name,
            std::make_unique<FixedArea>(name, network));
    }

    void decimate(std::string const& pg_nw_name,
                  JSON const& pg_network,
                  NameSet& accumulator,
                  JSON& decimated_jsons,
                  size_t& dec_id) {
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

            auto const& dec_name = pg_nw_name + ".dec-" + std::to_string(dec_id++);
            decimated_jsons[dec_name] = decimated_json;
            accumulator.clear();
        }
    }

    void simple_decimate(std::string const& pg_nw_name,
                         JSON const& pg_network)
    {
        JSON decimated_jsons;
        NameSet accumulator;
        size_t dec_id = 0;

        auto const call_decimate = [&]() {
            decimate(pg_nw_name,
                     pg_network,
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
            _.work_areas_.emplace(
                dec_name,
                std::make_unique<WorkArea>(dec_name, dec_json, _.fixed_areas_));

            PANIC_IF(_.work_areas_[dec_name]->joints.empty(),
                     ShouldNotReach("PathGuide network \"" + pg_nw_name +
                                    "\" has no joints associated. "
                                    "Error in parsing, maybe?"));
        }
    }

    void star_decimate(std::string const& pg_nw_name,
                       JSON const& pg_network,
                       AdjacentNames const& adj_leaves) {
        // - 1. Find compatible hubs.
        // - 2. Define 1H work area for each star arm.
        // - 3. Solve for each starting 1H work area to find best score, then
        //   solve for remaining 1H arms.
        throw ShouldNotReach("Good!");
    }

    void allorders_decimate(std::string const& pg_nw_name,
                            JSON const& pg_network,
                            AdjacentNames const& adj_bps,
                            AdjacentNames const& adj_leaves) {
        // - 1. Find compatible hubs per bp.
        // - 2. Create (bp_name ^ team cksm), (bp_name ^ team cksm) cache map.
        // - 3. Create initial segments.
        //
        //   ... Run later during solve_all() ...
        // - 4. Traverse edges in all neighboring orders and solve segments
        //   on-the-fly if not found in cache.
        throw ShouldNotReach("Bad!");
    }

    void digest_pg_network(std::string const & pg_nw_name,
                           JSON const & pg_network) {
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
                    star_decimate(pg_nw_name, pg_network, adj_leaves);
                }
                else {
                    allorders_decimate(pg_nw_name, pg_network, adj_bps, adj_leaves);
                }

                // There can only be one single hub network in a pg_network
                // since all UIJoints are connected.
                return;
            }
        }

        simple_decimate(pg_nw_name, pg_network);
    }
};

/* public */
/* ctors */
Spec::Spec(Options const& options) :
    pimpl_(std::make_unique<PImpl>(*this)) {
    pimpl_->parse(options);
}

Spec::Spec(Spec&& other) {
    this->operator=(std::move(other));
}

/* dtors */
Spec::~Spec() {}

/* modifiers */
Spec& Spec::operator=(Spec&& other) {
    if (this != &other) {
        work_areas_.clear();
        fixed_areas_.clear();

        std::swap(work_areas_, other.work_areas_);
        std::swap(fixed_areas_, other.fixed_areas_);
    }

    return *this;
}

void Spec::solve_all() {
    // Solve each work package.
    for (auto& [wa_name, wa_sp] : work_areas_) {
        wa_sp->solve();
    }
}

}  /* elfin */