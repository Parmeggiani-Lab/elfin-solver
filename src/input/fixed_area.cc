#include "fixed_area.h"

namespace elfin {

FixedArea::FixedArea(const JSON & j, const std::string & name) {
    for(auto it = j.begin(); it != j.end(); ++it) {
        modules_.emplace_back(*it, it.key());
    }
}

}  /* elfin */