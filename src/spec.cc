#include "spec.h"

#include "debug_utils.h"
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

    /* modifiers */
    void parse(Options const& options) {
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

            auto const& dec_name = name + ".dec-" + std::to_string(dec_id++);
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