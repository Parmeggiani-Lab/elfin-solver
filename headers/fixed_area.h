#ifndef FIXED_AREA_H_
#define FIXED_AREA_H_

#include <string>
#include <unordered_map>

#include "ui_object.h"

namespace elfin {

class FixedArea
{
protected:
    std::string name_;
    UIObjects modules_;

public:
    FixedArea(JSON const& j, const std::string& name);

    /* getters */
    std::string name() const { return name_; }
    UIObjects const& modules() const { return modules_; }
};

typedef std::unordered_map<std::string, FixedArea *> FixedAreaMap;

}  /* elfin */

#endif  /* end of include guard: FIXED_AREA_H_ */