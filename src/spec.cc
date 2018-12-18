#include "spec.h"

#include "debug_utils.h"

namespace elfin {

const char* const Spec::pg_networks_name = "pg_networks";
const char* const Spec::networks_name = "networks";

/* protected */
/* modifiers */
void Spec::map_joints() {
    for (auto itr : work_areas_) {
        WorkArea* wa = itr.second;
        for (UIJoint* oj : wa->occupied_joints()) {
            auto& occ_triple = oj->occupant_triple_;
            const std::string& occ_prnt_name = std::get<0>(occ_triple);
            const std::string& occ_name = std::get<1>(occ_triple);
            UIObjects const& modules = fixed_areas_.at(occ_prnt_name)->modules();
            UIObject const* occupant = &(modules.at(occ_name));

            std::get<2>(occ_triple) = occupant;
        }

        for (UIJoint* oj : wa->hinged_joints()) {
            auto& hinge_tuple = oj->hinge_tuple_;
            const std::string& hinge_name = std::get<0>(hinge_tuple);
            UIJoint const* hinge = &(wa->joints().at(hinge_name));

            std::get<1>(hinge_tuple) = hinge;
        }
    }
}

void Spec::release_resources() {
    for (auto itr : work_areas_) {
        delete itr.second;
    }
    work_areas_.clear();

    for (auto itr : fixed_areas_) {
        delete itr.second;
    }
    fixed_areas_.clear();
}

/* public */
/* dtors */
Spec::~Spec() {
    release_resources();
}

/* modifiers */
void Spec::parse_from_json(JSON const& j) {
    try {
        release_resources();

        JSON const pg_networks = j.at("pg_networks");
        JSON const networks = j.at("networks");

        dbg("Input spec has %d work areas\n", pg_networks.size());
        for (auto it = pg_networks.begin(); it != pg_networks.end(); ++it) {
            std::string const& jt_name = it.key();
            work_areas_.emplace(jt_name, new WorkArea(*it, jt_name));

            NICE_PANIC(work_areas_[jt_name]->joints().empty(),
                       string_format("Work area \"%s\" has no joints associated.",
                                     it.key().c_str()));
        }

        for (auto it = networks.begin(); it != networks.end(); ++it) {
            std::string const& fa_name = it.key();
            fixed_areas_.emplace(fa_name, new FixedArea(*it, fa_name));
        }
    } catch (std::exception const& e) {
        NICE_PANIC("Exception",
                   string_format("Failed to parse spec from JSON."
                                 "\nReason: %s", e.what()));
    }

    map_joints();
}

}  /* elfin */