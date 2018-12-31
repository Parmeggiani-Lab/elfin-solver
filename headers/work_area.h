#ifndef WORK_AREA_H_
#define WORK_AREA_H_

#include <unordered_map>
#include <string>
#include <memory>

#include "ui_joint.h"
#include "geometry.h"
#include "fixed_area.h"

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
    MACRO(NONE) \

GEN_ENUM_AND_STRING(WorkType, WorkTypeNames, FOREACH_WORKTYPE);

void bad_work_type(WorkType type);

struct WorkArea {
    /* data */
    std::string const   name;
    UIJointMap const    joints;
    UIJointKeys const   occupied_joints;
    UIJointKeys const   leaf_joints;
    WorkType const      type;
    V3fList const       points;
    size_t const        target_size;

    /* ctors */
    WorkArea(
        std::string const& _name,
        JSON const& json,
        FixedAreaMap const& fam);

    /* dtors */
    virtual ~WorkArea();
};

typedef SPMap<WorkArea> WorkAreaMap;

}  /* elfin */

#endif  /* end of include guard: WORK_AREA_H_ */