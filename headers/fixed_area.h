#ifndef FIXED_AREA_H_
#define FIXED_AREA_H_

#include <memory>
#include <string>
#include <vector>

#include "ui_object.h"

namespace elfin {

class FixedArea
{
protected:
    UIObjects modules_;

public:
    const std::string name_;

    FixedArea(const JSON & j, const std::string & name);
};

typedef std::vector<FixedArea> FixedAreas;

}  /* elfin */

#endif  /* end of include guard: FIXED_AREA_H_ */