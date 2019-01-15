#include "spec.h"

#include "debug_utils.h"
#include "database.h"
#include "options.h"
#include "json.h"

namespace elfin {

/* private */
struct Spec::PImpl {
    /* types */
    // Maps hub ProtoModule key -> vector of Transforms (orientations).
    typedef std::unordered_map<PtModKey, std::vector<Transform>> HubOrientations;

    /* data */
    Spec& _;

    /* ctors */
    PImpl(Spec& interface) : _(interface) { }

    void digest_network(std::string const& name, JSON const& json) {
        _.fixed_areas_.emplace(
            name,
            std::make_unique<FixedArea>(name, json));
    }

    void digest_pg_network(std::string const& name, JSON const& json) {
        JSON decimated_jsons;

        // First, compute hub allocation map for each branchpoint.
        // Maps joint UI name -> hub orientations.
        std::unordered_map<std::string, HubOrientations> hub_alloc_map;
        for (auto const& [joint_name, joint_json] : json.items()) {
            if (joint_json.at("neighbors").size() > 2) {
                //hub_alloc_list...
                UNIMP();
                // Break it down to multiple simple ones by first choosing hubs
                // and their orientations.
                /*
                TODO: Break complex work area
                */
            }
        }

        std::unordered_set<std::string> accumulator;
        size_t dec_id = 0;

        auto const decimate = [&]() {
            PANIC_IF(accumulator.empty(), ShouldNotReach("Nothing to decimate...?"));

            JSON decimated_json;
            // Trim accumulator joints by removing neighbor names that aren't
            // in this set.
            for (auto const& joint_name : accumulator) {
                decimated_json[joint_name] = json[joint_name];  // Make mutable copy.

                auto& nbs = decimated_json[joint_name].at("neighbors");
                nbs.erase(std::remove_if(begin(nbs), end(nbs),
                [&accumulator](auto const & nb_name) {
                    return accumulator.find(nb_name) == end(accumulator);
                }),
                end(nbs));
            }

            auto const& dec_name = name + ".dec" + std::to_string(dec_id++);
            decimated_jsons[dec_name] = decimated_json;
            accumulator.clear();
        };

        // Second, decimate the pg_network into a bunch of segments. A segment start
        // and end at either a leaf joint or a hinge (occupied) joint.
        for (auto const& [joint_name, joint_json] : json.items()) {
            accumulator.insert(joint_name);

            if (joint_json.at("occupant") != "" and
                    joint_json.at("neighbors").size() > 1) {
                // This is a non-leaf joint that is occupied, therefore we
                // need to break the pg_network.
                decimate();

                // Re-insert current joint, which will be the beginning of the
                // next decimated segment.
                accumulator.insert(joint_name);
            }
        }

        if (not accumulator.empty()) {
            decimate();
        }

        for (auto const& [dec_name, dec_json] : decimated_jsons.items()) {
            _.work_areas_.emplace(
                dec_name,
                std::make_unique<WorkArea>(dec_name, dec_json, _.fixed_areas_));

            PANIC_IF(_.work_areas_[dec_name]->joints.empty(),
                     ShouldNotReach("Work area \"" + name +
                                    "\" has no joints associated. "
                                    "Error in parsing, maybe?"));
        }
    }
};

/* public */
/* ctors */
Spec::Spec() :
    pimpl_(std::make_unique<PImpl>(*this)) {}

/* dtors */
Spec::~Spec() {}

/* modifiers */
void Spec::parse(Options const& options) {
    work_areas_.clear();
    fixed_areas_.clear();

    PANIC_IF(options.spec_file.empty(),
             BadArgument("No input spec file provided.\n"));

    PANIC_IF(not JUtil.file_exists(options.spec_file.c_str()),
             BadArgument("Input file \"" + options.spec_file + "\" does not exist.\n"));

    JSON const& spec_json = parse_json(options.spec_file);
    try {
        JUtil.info("Input spec has %zu pg_networks and %zu networks\n",
                   spec_json.at("pg_networks").size(),
                   spec_json.at("networks").size());

        // Initialize fixed areas first so work areas can refer to fixed
        // modules as occupants or hinges.
        for (auto& [name, json] : spec_json.at("networks").items()) {
            pimpl_->digest_network(name, json);
        }

        for (auto& [name, json] : spec_json.at("pg_networks").items()) {
            pimpl_->digest_pg_network(name, json);
        }
    } catch (JSON::exception const& je) {
        JSON_LOG_EXIT(je);
    }
}

}  /* elfin */