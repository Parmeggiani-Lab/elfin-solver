#ifndef WORK_AREA_H_
#define WORK_AREA_H_

#include "../../data/Geometry.h"

namespace elfin {

/*
 - FREE: a string-like path guide with no hinge on either side.
 - ONE_HINGE: like FREE, but one end is hinged.
 - TWO_HINGE: like ONE_HINGE, but ther other end is also hinged.
 - COMPLEX: a path guide that has at least one branch point with more than
   2 bridges. Will need to be broken down into the previous three types after
   hub assignment.
*/
enum WorkType { FREE, ONE_HINGE, TWO_HINGE, COMPLEX };

// TODO: Remove inheritance
class WorkArea : public Points3f
{
protected:
    WorkType type_ = FREE;

public:
    WorkArea() {
        // place holder
        for (int i = 0; i < 5; i++)
            this->emplace_back(10 * i, 15 * i, 20 * i);
    };
    virtual ~WorkArea() {};

    WorkType get_type() const { return type_; };
};

}  /* elfin */

#endif  /* end of include guard: WORK_AREA_H_ */