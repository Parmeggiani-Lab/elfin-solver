#include "fixed_area.h"

namespace elfin {

UIObjectMap parse_modules(JSON const& json) {
    UIObjectMap res;
    for (auto& [mod_name , mod_json] : json.items()) {
        res.emplace(
            mod_name,
            std::make_unique<UIObject>(mod_name, mod_json));
    }
    return res;
}

FixedArea::FixedArea(std::string const& _name, JSON const& json) :
    name(_name),
    modules(parse_modules(json)) {}

}  /* elfin */