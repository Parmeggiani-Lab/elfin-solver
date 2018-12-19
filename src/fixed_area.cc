#include "fixed_area.h"

namespace elfin {

FixedArea::FixedArea(JSON const& json, const std::string& name) :
    name_(name) {
    for (auto it = begin(json); it != end(json); ++it) {
        std::string mod_name = it.key();
        modules_.emplace(
            mod_name,
            std::make_shared<UIObject>(*it, mod_name));
    }
}

}  /* elfin */