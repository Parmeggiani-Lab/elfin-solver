#include "spec.h"

#include "debug_utils.h"

namespace elfin {

/* public */
/* modifiers */
void Spec::parse_from_json(JSON const& j) {
    try {
        work_areas_.clear();
        fixed_areas_.clear();

        JSON const pg_networks = j.at("pg_networks");
        JSON const networks = j.at("networks");

        msg("Input spec has %lu work areas and %lu fixed areas\n",
            pg_networks.size(),
            networks.size());

        // Initialize fixed areas first so work areas can refer to fixed
        // modules as occupants or hinges.
        for (auto it = networks.begin(); it != networks.end(); ++it) {
            std::string const& network_name = it.key();
            fixed_areas_.emplace(
                network_name,
                std::make_unique<FixedArea>(*it, network_name));
        }

        for (auto it = pg_networks.begin(); it != pg_networks.end(); ++it) {
            std::string const& pgn_name = it.key();
            work_areas_.emplace(
                pgn_name,
                std::make_unique<WorkArea>(*it, pgn_name, fixed_areas_));

            NICE_PANIC(
                work_areas_[pgn_name]->joints().empty(),
                string_format(
                    "Work area \"%s\" has no joints associated.",
                    pgn_name.c_str()));
        }
    } catch (std::exception const& e) {
        NICE_PANIC("Exception",
                   string_format("Failed to parse spec from JSON."
                                 "\nReason: %s", e.what()));
    }
}

}  /* elfin */