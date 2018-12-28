#include "spec.h"

#include "debug_utils.h"

namespace elfin {

/* public */
/* modifiers */
void Spec::parse_from_json(JSON const& json) {
    try {
        work_areas_.clear();
        fixed_areas_.clear();

        JSON const& pg_networks = json.at("pg_networks");
        JSON const& networks = json.at("networks");

        JUtil.info("Input spec has %zu work areas and %zu fixed areas\n",
            pg_networks.size(),
            networks.size());

        // Initialize fixed areas first so work areas can refer to fixed
        // modules as occupants or hinges.
        for (auto it = begin(networks);
                it != end(networks);
                ++it) {
            std::string const& network_name = it.key();
            fixed_areas_.emplace(
                network_name,
                std::make_unique<FixedArea>(*it, network_name));
        }

        for (auto it = begin(pg_networks);
                it != end(pg_networks);
                ++it) {
            std::string const& pg_network_name = it.key();

            // If this was a complex work area, we need to break it down to
            // multiple simple ones by first choosing hubs and their orientations.
            /*
            TODO: Break complex work area
            */
            work_areas_.emplace(
                pg_network_name,
                std::make_unique<WorkArea>(pg_network_name, *it, fixed_areas_));

            TRACE_PANIC(
                work_areas_[pg_network_name]->joints().empty(),
                string_format(
                    "Work area \"%s\" has no joints associated.",
                    pg_network_name.c_str()));
        }
    } catch (std::exception const& e) {
        TRACE_PANIC("Exception",
                   string_format("Failed to parse spec from JSON."
                                 "\nReason: %s", e.what()));
    }
}

}  /* elfin */