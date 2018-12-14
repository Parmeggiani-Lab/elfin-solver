#include "spec.h"

#include "debug_utils.h"

namespace elfin {

const char* const Spec::pg_networks_name = "pg_networks";
const char* const Spec::networks_name = "networks";

Spec::Spec(const JSON& j) {
    parse_from_json(j);
}

void Spec::parse_from_json(const JSON& j) {
    try {
        const JSON pg_networks = j.at("pg_networks");
        const JSON networks = j.at("networks");

        dbg("Input spec has %d work areas\n", pg_networks.size());
        for (auto it = pg_networks.begin(); it != pg_networks.end(); ++it) {
            std::string jt_name = it.key();
            auto wa_itr = work_area_map_.emplace(jt_name, WorkArea(*it, jt_name));
            auto& key_val = *wa_itr.first;
            const WorkArea& wa = key_val.second;

            NICE_PANIC(work_area_map_[jt_name].joints().empty(),
                       string_format("Work area \"%s\" has no joints associated.",
                                     it.key().c_str()));
        }

        for (auto it = networks.begin(); it != networks.end(); ++it) {
            std::string fa_name = it.key();
            fixed_areas_.emplace(fa_name, FixedArea(*it, fa_name));
        }
    } catch (const std::exception& e) {
        NICE_PANIC("Exception",
                   string_format("Failed to parse spec from JSON."
                                 "\nReason: %s", e.what()));
    }

    map_joints();
}

void Spec::map_joints() {
    for (auto& itr : work_area_map_) {
        WorkArea& wa = itr.second;
        for (UIJoint* oj : wa.occupied_joints()) {
            auto& occ_triple = oj->occupant_triple_;
            const std::string& occ_prnt_name = std::get<0>(occ_triple);
            const std::string& occ_name = std::get<1>(occ_triple);
            UIObjects const& modules = fixed_areas_.at(occ_prnt_name).modules();
            UIObject const* occupant = &(modules.at(occ_name));

            std::get<2>(occ_triple) = occupant;
        }

        for (UIJoint* oj : wa.hinged_joints()) {
            auto& hinge_tuple = oj->hinge_tuple_;
            const std::string& hinge_name = std::get<0>(hinge_tuple);
            UIJoint const* hinge = &(wa.joints().at(hinge_name));

            std::get<1>(hinge_tuple) = hinge;
        }
    }
}

}  /* elfin */