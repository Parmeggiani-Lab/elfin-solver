#include "spec_parser.h"

#include "elfin_exception.h"
#include "jutil.h"

namespace elfin {

const char * const SpecParser::pg_networks_name = "pg_networks";
const char * const SpecParser::networks_name = "networks";

/* Public Methods */
std::shared_ptr<Spec> SpecParser::parse(const std::string & filepath) {

    try {
        const JSON j = parse_json(filepath);
        std::shared_ptr<Spec> spec = std::make_shared<Spec>();

        const JSON pg_networks = j.at("pg_networks");
        const JSON networks = j.at("networks");

        msg("Input spec has %d work areas\n", pg_networks.size());
        for (auto it = pg_networks.begin(); it != pg_networks.end(); ++it) {
            spec->work_areas_.emplace_back(*it, it.key());

            if (spec->work_areas_.back().joints().size() == 0) {
                throw ElfinException("Work area \"" + it.key() + "\" has no joints associated.");
            }
        }

        for (auto it = networks.begin(); it != networks.end(); ++it) {
            spec->fixed_areas_.emplace_back(*it, it.key());
        }

        return spec;
    } catch (const std::exception & e) {
        err("Failed to parse spec from JSON.\nReason: %s\n", e.what());
        return nullptr;
    }
}

} /* elfin */