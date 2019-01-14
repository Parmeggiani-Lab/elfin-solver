#include "ui_joint.h"

#include "debug_utils.h"

namespace elfin {

/* occupant */
/* printers */
void UIJoint::Occupant::print_to(std::ostream& os) const {
    if (ui_module) {
        os << "Occupant [\n";
        os << "  parent_name: " << parent_name << "\n";
        os << "  ui_module: " << *ui_module << "\n";
        os << "]";
    }
    else {
        os << "Occupant [ empty ]";
    }
}

StrList parse_neighbors(JSON const& json) {
    StrList res;

    try {
        for (auto neighbor_name : json.at("neighbors")) {
            res.push_back(neighbor_name);
        }
    } catch (JSON::exception const& je) {
        JSON_LOG_EXIT(je);
    }

    return res;
}

UIJoint::Occupant parse_occupant(JSON const& json,
                                 FixedAreaMap const& fam) {
    UIJoint::Occupant res;

    try {
        if (json.at("occupant") != "") {
            std::string const& occ_network = json.at("occupant_parent");
            auto& fxn_modules = fam.at(occ_network)->modules;

            res.parent_name = occ_network;
            res.ui_module = fxn_modules.at(json.at("occupant")).get();
        }
    } catch (JSON::exception const& je) {
        JSON_LOG_EXIT(je);
    }

    return res;
}

std::string parse_hinge_name(JSON const& json) {
    std::string res;

    try {
        res = json.at("hinge");
    } catch (JSON::exception const& je) {
        JSON_LOG_EXIT(je);
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
    os << "Neighbors:\n";
    for (auto& nb : neighbors) {
        os << "  " << nb << "\n";
    }
    os << occupant << "\n";
    os << "hinge: " << hinge_name << "\n";
    os << "]";
}

}  /* elfin */