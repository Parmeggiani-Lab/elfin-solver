#include "spec.h"

#include "debug_utils.h"

namespace elfin {

/* public */
/* modifiers */
void Spec::parse_from_json(JSON const& json) {
    try {
        work_areas_.clear();
        fixed_areas_.clear();

        JUtil.info("Input spec has %zu work areas and %zu fixed areas\n",
                   json.at("pg_networks").size(),
                   json.at("networks").size());

        // Initialize fixed areas first so work areas can refer to fixed
        // modules as occupants or hinges.
        for (auto& [name, json] : json.at("networks").items()) {
            fixed_areas_.emplace(
                name,
                std::make_unique<FixedArea>(json, name));
        }

        for (auto& [name, json] : json.at("pg_networks").items()) {
            // If this was a complex work area, we need to break it down to
            // multiple simple ones by first choosing hubs and their orientations.
            /*
            TODO: Break complex work area
            */
            work_areas_.emplace(
                name,
                std::make_unique<WorkArea>(name, json, fixed_areas_));

            TRACE(work_areas_[name]->joints().empty(),
                  "Work area \"%s\" has no joints associated.",
                  name.c_str());
        }
    } catch (std::exception const& e) {
        TRACE("Failed to parse spec from JSON.", "%s\n", e.what());
    }
}

}  /* elfin */