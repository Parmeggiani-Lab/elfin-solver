#ifndef WORK_AREA_H_
#define WORK_AREA_H_

#include "ui_object.h"
#include "geometry.h"
#include "jutil.h"

namespace elfin {

/*
 - FREE: a string-like path guide with no hinge on either side.
 - ONE_HINGE: like FREE, but one end is hinged.
 - TWO_HINGE: like ONE_HINGE, but ther other end is also hinged.
 - COMPLEX: a path guide that has at least one branch point with more than
   2 bridges. Will need to be broken down into the previous three types after
   hub assignment.
*/

#define FOREACH_WORKTYPE(MACRO) \
    MACRO(FREE) \
    MACRO(ONE_HINGE) \
    MACRO(TWO_HINGE) \
    MACRO(COMPLEX) \

GEN_ENUM_AND_STRING(WorkType, WorkTypeNames, FOREACH_WORKTYPE);

// TODO: Remove inheritance - or not?
class WorkArea : public Points3f, public UIObject<WorkArea>
{
protected:
    WorkType type_ = FREE;

public:
    using UIObject::UIObject;

    static std::shared_ptr<WorkArea> from_json(const JSON & j, const std::string & name);
    WorkType get_type() const { return type_; };
};

typedef std::vector<std::shared_ptr<const WorkArea>> WorkAreas;

}  /* elfin */

#endif  /* end of include guard: WORK_AREA_H_ */