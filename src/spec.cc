#include "spec.h"

#include "debug_utils.h"

namespace elfin {

/* public */
/* modifiers */
void Spec::parse_from_json(JSON const& json) {
    work_areas_.clear();
    fixed_areas_.clear();

    try {
        JUtil.info("Input spec has %zu work areas and %zu fixed areas\n",
                   json.at("pg_networks").size(),
                   json.at("networks").size());

        // Initialize fixed areas first so work areas can refer to fixed
        // modules as occupants or hinges.
        for (auto& [name, nw_json] : json.at("networks").items()) {
            fixed_areas_.emplace(
                name,
                std::make_unique<FixedArea>(name, nw_json));
        }

        for (auto& [name, pgnw_json] : json.at("pg_networks").items()) {
            size_t const num_branch_points =
                std::accumulate(
                    begin(pgnw_json),
                    end(pgnw_json),
                    0,
            [](size_t sum, auto & joint_json) {
                return sum + (joint_json["neighbors"].size() > 2);
            });

            if (num_branch_points > 2) {
                UNIMP();
                // Break it down to multiple simple ones by first choosing hubs
                // and their orientations.
                /*
                TODO: Break complex work area
                */
            }

            JUtil.warn("num_branch_points=%zu\n", num_branch_points);

            work_areas_.emplace(
                name,
                std::make_unique<WorkArea>(name, pgnw_json, fixed_areas_));

            TRACE(work_areas_[name]->joints.empty(),
                  "Work area \"%s\" has no joints associated.",
                  name.c_str());
        }
    } catch (std::exception const& e) {
        TRACE("Failed to parse spec from JSON.", "%s\n", e.what());
    }
}

}  /* elfin */