#include "spec.h"

#include "debug_utils.h"
#include "database.h"
#include "options.h"
#include "json.h"

namespace elfin {

/* private */
struct Spec::PImpl {
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
        size_t const num_branch_points =
            std::accumulate(begin(json), end(json), 0,
        [](size_t sum, auto & joint_json) {
            return sum + (joint_json.at("neighbors").size() > 2);
        });

        if (num_branch_points > 0) {
            UNIMP();
            // Break it down to multiple simple ones by first choosing hubs
            // and their orientations.
            /*
            TODO: Break complex work area
            */
        }

        _.work_areas_.emplace(
            name,
            std::make_unique<WorkArea>(name, json, _.fixed_areas_));

        PANIC_IF(_.work_areas_[name]->joints.empty(),
                 ShouldNotReach("Work area \"" + name +
                                "\" has no joints associated. Error in parsing, maybe?"));
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
        JUtil.info("Input spec has %zu work areas and %zu fixed areas\n",
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