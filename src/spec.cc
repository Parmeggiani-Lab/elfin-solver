#include "spec.h"

#include "debug_utils.h"
#include "database.h"
#include "options.h"
#include "json.h"

namespace elfin {

/* public */
/* modifiers */
void Spec::parse(Options const& options) {
    work_areas_.clear();
    fixed_areas_.clear();

    PANIC_IF(options.spec_file.empty(), "No input spec file provided.\n");

    PANIC_IF(not JUtil.file_exists(options.spec_file.c_str()),
             "Input file does not exist.\n");

    JSON const& spec_json = parse_json(options.spec_file);
    try {
        JUtil.info("Input spec has %zu work areas and %zu fixed areas\n",
                   spec_json.at("pg_networks").size(),
                   spec_json.at("networks").size());

        // Initialize fixed areas first so work areas can refer to fixed
        // modules as occupants or hinges.
        for (auto& [name, json] : spec_json.at("networks").items()) {
            fixed_areas_.emplace(
                name,
                std::make_unique<FixedArea>(name, json));
        }

        for (auto& [name, json] : spec_json.at("pg_networks").items()) {
            size_t const num_branch_points =
                std::accumulate(
                    begin(json),
                    end(json),
                    0,
            [](size_t sum, auto & joint_json) {
                return sum + (joint_json["neighbors"].size() > 2);
            });

            if (num_branch_points > 0) {
                UNIMP();
                // Break it down to multiple simple ones by first choosing hubs
                // and their orientations.
                /*
                TODO: Break complex work area
                */
            }

            work_areas_.emplace(
                name,
                std::make_unique<WorkArea>(name, json, fixed_areas_));

            TRACE(work_areas_[name]->joints.empty(),
                  "Work area \"%s\" has no joints associated.",
                  name.c_str());
        }
    } catch (std::exception const& e) {
        TRACE("Failed to parse spec from JSON.", "%s\n", e.what());
    }
}

}  /* elfin */