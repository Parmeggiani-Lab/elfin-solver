#include "spec.h"

#include "jutil.h"

namespace elfin {

const char * const Spec::pg_networks_name = "pg_networks";
const char * const Spec::networks_name = "networks";

Spec::Spec(const JSON & j) {
    parse_from_json(j);
}

void Spec::parse_from_json(const JSON & j) {
    try {
        const JSON pg_networks = j.at("pg_networks");
        const JSON networks = j.at("networks");

        dbg("Input spec has %d work areas\n", pg_networks.size());
        for (auto it = pg_networks.begin(); it != pg_networks.end(); ++it) {
            std::string jt_name = it.key();
            auto wa_itr = work_areas_.emplace(jt_name, WorkArea(*it, jt_name));
            auto & key_val = *wa_itr.first;
            const WorkArea & wa = key_val.second;

            if (wa.joints().size() == 0) {
                throw ElfinException("Work area \"" + it.key() + "\" has no joints associated.");
            }
        }

        for (auto it = networks.begin(); it != networks.end(); ++it) {
            std::string fa_name = it.key();
            fixed_areas_.emplace(fa_name, FixedArea(*it, fa_name));
        }
    } catch (const std::exception & e) {
        err("Failed to parse spec from JSON.\nReason: %s\n", e.what());
        throw e;
    }

    map_occupied_joints();
}

void Spec::map_occupied_joints() {
    for (auto & itr : work_areas_) {
        WorkArea & wa = itr.second;
        auto & oj = wa.occupied_joints();
        if (oj.size()) {

        }
    }
}

}  /* elfin */