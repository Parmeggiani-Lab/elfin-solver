#include "ui_module.h"

namespace elfin {

std::string parse_module_name(JSON const& json) {
    std::string res;
    try {
        res = json["module_name"];
    } catch (const std::exception& e) {
        TRACE("Failed to parse spec from JSON.", "%s\n", e.what());
    }
    return res;
}

UIModule::UIModule(std::string const& name,
                   JSON const& json) :
    UIObject(name, json),
    module_name(parse_module_name(json)) {}

/* printers */
void UIModule::print_to(std::ostream& os) const {
    os << "UIModule (" << name << ") [\n";
    os << "  module_name: " << module_name << "\n";
    os << "]";
}

}  /* elfin */