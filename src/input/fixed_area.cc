#include "fixed_area.h"

namespace elfin {

FixedArea::FixedArea(const JSON & j, const std::string & name) :
    name_(name) {
    for (auto it = j.begin(); it != j.end(); ++it) {
        std::string mod_name = it.key();
        modules_.emplace(mod_name, UIObject(*it, mod_name));
    }
}

}  /* elfin */