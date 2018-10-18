#ifndef WORK_AREA_H_
#define WORK_AREA_H_

#include <memory>

#include "../../data/Geometry.h"
#include "json.h"

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
    WorkArea() {};
    virtual ~WorkArea() {};

    static std::shared_ptr<WorkArea> from_json(const JSON & pgn, const JSON & networks);
    WorkType get_type() const { return type_; };
};

typedef std::vector<std::shared_ptr<const WorkArea>> WorkAreas;

}  /* elfin */

#endif  /* end of include guard: WORK_AREA_H_ */