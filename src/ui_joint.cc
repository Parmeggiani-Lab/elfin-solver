#include "ui_joint.h"

#include "debug_utils.h"

namespace elfin {

/* occupant */
/* printers */
void UIJoint::Occupant::print_to(std::ostream& os) const {
    os << "Occupant (" << (name == "" ? "null" : name) << ") [\n";
    os << "  parent_name: " << parent_name << "\n";
    os << "  module: " << (module ? module->name : "null") << "\n";
    os << "]";
}

StrList parse_neighbors(JSON const& json) {
    StrList res;

    try {
        for (auto neighbor_name : json["neighbors"]) {
            res.push_back(neighbor_name);
        }
    } catch (const std::exception& e) {
        TRACE("Failed to parse spec from JSON.", "%s\n", e.what());
    }

    return res;
}

UIJoint::Occupant parse_occupant(JSON const& json,
                                 FixedAreaMap const& fam) {
    UIJoint::Occupant res;

    try {
        res.name = json["occupant"];
        if (res.name != "") {
            std::string const& occ_network = json["occupant_parent"];
            auto& fxn_modules = fam.at(occ_network)->modules;

            res.parent_name = occ_network;
            res.module = fxn_modules.at(res.name).get();
        }
    } catch (const std::exception& e) {
        TRACE("Failed to parse spec from JSON.", "%s\n", e.what());
    }

    return res;
}

std::string parse_hinge_name(JSON const& json) {
    std::string res;

    try {
        res = json["hinge"];
    } catch (const std::exception& e) {
        TRACE("Failed to parse spec from JSON.", "%s\n", e.what());
    }

    return res;
}

UIJoint::UIJoint(std::string const& name,
                 JSON const& json,
                 FixedAreaMap const& fam) :
    UIObject(name, json),
    neighbors(parse_neighbors(json)),
    occupant(parse_occupant(json, fam)),
    hinge_name(parse_hinge_name(json)) {}

/* printers */
void UIJoint::print_to(std::ostream& os) const {
    os << "UIJoint (" << name << ") [\n";
    os << tx << "\n";
    os << "neighbors:\n";
    for (auto& nb : neighbors) {
        os << "  " << nb << "\n";
    }
    os << occupant << "\n";
    os << "hinge: " << hinge_name << "\n";
    os << "]";
}

}  /* elfin */