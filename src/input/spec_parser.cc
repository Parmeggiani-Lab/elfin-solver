#include "spec_parser.h"

#include "../../jutil/src/jutil.h"

namespace elfin {

const char * const SpecParser::pg_networks_name = "pg_networks";
const char * const SpecParser::networks_name = "networks";

std::shared_ptr<Spec> SpecParser::parse(const std::string & filepath) {
    std::shared_ptr<Spec> spec = nullptr;

    const JSON j = parse_json(filepath);
    spec = std::make_shared<Spec>();

    try {
        const JSON pg_networks = j.at("pg_networks");
        const JSON networks = j.at("networks");

        msg("Input spec has %d work areas\n", pg_networks.size());
        for(auto pgn : pg_networks) {
            WorkArea * wa = new WorkArea();

            // TODO: Fill work area with pgn data

            spec->work_areas_.push_back(wa);
        }
    } catch (const std::exception & e) {
        err("Failed to parse spec from JSON.\nReason: %s\n", e.what());
        return nullptr;
    }

    return spec;
}

} /* elfin */