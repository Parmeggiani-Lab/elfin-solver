#ifndef SPEC_H_
#define SPEC_H_

#include <vector>

#include "work_area.h"

namespace elfin {

class SpecParser;

class Spec
{
    friend class SpecParser;

protected:
    std::vector<const WorkArea *> work_areas_;

public:
    Spec() {};
    virtual ~Spec() {
        while(work_areas_.size() > 0) {
            const WorkArea * wa = work_areas_.back();
            work_areas_.pop_back();
            delete wa;
        }
    }

    const std::vector<const WorkArea *> & get_work_areas() const { return work_areas_; };
};

}  /* elfin */

#endif  /* end of include guard: SPEC_H_ */