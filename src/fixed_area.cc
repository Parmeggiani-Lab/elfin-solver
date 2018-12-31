#include "fixed_area.h"

namespace elfin {

UIModuleMap parse_modules(JSON const& json) {
    UIModuleMap res;
    for (auto& [mod_name , mod_json] : json.items()) {
        res.emplace(
            mod_name,
            std::make_unique<UIModule>(mod_name, mod_json));
    }
    return res;
}

FixedArea::FixedArea(std::string const& _name, JSON const& json) :
    name(_name),
    modules(parse_modules(json)) {}

}  /* elfin */