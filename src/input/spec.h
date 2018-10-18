#ifndef SPEC_H_
#define SPEC_H_

#include <vector>
#include <memory>

#include "work_area.h"

namespace elfin {

class SpecParser;

class Spec
{
    friend SpecParser;

protected:
    WorkAreas work_areas_;
    
public:
    Spec() {};
    virtual ~Spec() {};

    const WorkAreas & get_work_areas() const { return work_areas_; };
};

}  /* elfin */

#endif  /* end of include guard: SPEC_H_ */