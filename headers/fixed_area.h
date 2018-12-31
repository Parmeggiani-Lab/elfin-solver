#ifndef FIXED_AREA_H_
#define FIXED_AREA_H_

#include <string>

#include "ui_module.h"
#include "map_utils.h"

namespace elfin {

struct FixedArea
{
    std::string const name;
    UIModuleMap const modules;

    FixedArea(std::string const& _name, JSON const& json);
};

typedef SPMap<FixedArea> FixedAreaMap;

}  /* elfin */

#endif  /* end of include guard: FIXED_AREA_H_ */