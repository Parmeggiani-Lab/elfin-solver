#ifndef FIXED_AREA_H_
#define FIXED_AREA_H_

#include <string>

#include "ui_object.h"
#include "map_utils.h"

namespace elfin {

class FixedArea
{
protected:
    std::string name_;
    UIObjectMap modules_;

public:
    FixedArea(JSON const& json, std::string const& name);

    /* getters */
    std::string name() const { return name_; }
    UIObjectMap const& modules() const { return modules_; }
};

typedef SPMap<FixedArea> FixedAreaMap;

}  /* elfin */

#endif  /* end of include guard: FIXED_AREA_H_ */