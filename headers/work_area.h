#ifndef WORK_AREA_H_
#define WORK_AREA_H_

#include <unordered_map>
#include <string>

#include "ui_joint.h"
#include "geometry.h"

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

inline void bad_work_type(WorkType type) {
    NICE_PANIC(type == type, string_format("Bad WorkType: %s\n",
                                           WorkTypeToCStr(type)));
}

class WorkArea
{
protected:
    std::string name_ = "unamed_area";
    WorkType type_ = WorkType::FREE;
    UIJointMap joints_;
    std::vector<UIJoint *> occupied_joints_;
    std::vector<UIJoint *> hinged_joints_;
    std::vector<UIJoint *> leaf_joints_;

public:
    WorkArea(JSON const& j, const std::string& name);
    WorkArea() {}
    V3fList to_points() const;

    /* getters */
    std::string name() const { return name_; }
    WorkType type() const { return type_; }
    UIJointMap const& joints() const { return joints_; }
    const std::vector<UIJoint *> & occupied_joints() const { return occupied_joints_; }
    const std::vector<UIJoint *> & hinged_joints() const { return hinged_joints_; }
};

typedef std::unordered_map<std::string, WorkArea> WorkAreaMap;

}  /* elfin */

#endif  /* end of include guard: WORK_AREA_H_ */