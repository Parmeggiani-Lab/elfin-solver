#include "fixed_area.h"

namespace elfin {

FixedArea::FixedArea(JSON const& json, std::string const& name) :
    name_(name) {
    for (auto& [mod_name , mod_json] : json.items()) {
        modules_.emplace(
            mod_name,
            std::make_unique<UIObject>(mod_json, mod_name));
    }
}

}  /* elfin */