#include "ui_joint.h"

#include "debug_utils.h"

namespace elfin {

UIJoint::UIJoint(
    JSON const& json,
    std::string const& name,
    FixedAreaMap const& fam) :
    UIObject(json, name) {
    try {
        // Parse neighbor names
        for (auto nb_json : json["neighbors"]) {
            neighbors_.push_back(nb_json);
        }

        // Parse occupant data
        std::string const& occupant_name = json["occupant"];
        if (occupant_name != "") {
            std::string const& occupant_network = json["occupant_parent"];
            auto& on_modules = fam.at(occupant_network)->modules();

            occupant_.parent_name = occupant_network;
            occupant_.name = occupant_name;
            occupant_.module = on_modules.at(occupant_name).get();
        }

        hinge_name_ = json["hinge"];
    } catch (const std::exception& e) {
        TRACE("Failed to parse spec from JSON.", "%s\n", e.what());
    }
}


}  /* elfin */