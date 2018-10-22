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
    FixedArea(const JSON & j, const std::string & name);

    /* getters */
    std::string name() const { return name_; }
    const UIObjects & modules() const { return modules_; }
};

typedef std::unordered_map<std::string, FixedArea> FixedAreas;

}  /* elfin */

#endif  /* end of include guard: FIXED_AREA_H_ */