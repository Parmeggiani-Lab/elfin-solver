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

class WorkArea {
public:
    /* ctors */
    WorkArea(
        JSON const& json,
        std::string const& name,
        FixedAreaMap const& fam);
    WorkArea() = delete;
    
    /* dtors */
    virtual ~WorkArea();

    /* accessors */
    std::string name() const;
    WorkType type() const;
    UIJointMap const& joints() const;
    size_t target_size() const;
    V3fList to_points() const;

private:
    struct PImpl;
    std::unique_ptr<PImpl> p_impl_;
};

typedef UPMap<WorkArea> WorkAreaMap;

}  /* elfin */

#endif  /* end of include guard: WORK_AREA_H_ */