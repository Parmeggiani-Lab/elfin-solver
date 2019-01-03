#ifndef WORK_AREA_H_
#define WORK_AREA_H_

#include <unordered_map>
#include <string>
#include <memory>
// #include <string>

#include "ui_joint.h"
#include "geometry.h"
#include "fixed_area.h"

namespace elfin {

/* Fwd Decl */
struct TestStat;

/* types */
#define FOREACH_WORKTYPE(MACRO) \
    MACRO(NONE) \
    MACRO(FREE) \
    MACRO(HINGED) \
    MACRO(DOUBLE_HINGED) \
    MACRO(_ENUM_SIZE)

GEN_ENUM_AND_STRING(WorkType, WorkTypeNames, FOREACH_WORKTYPE);

void bad_work_type(WorkType type);

struct WorkArea {
    /* types */
    typedef std::unordered_map<UIJointKey, V3fList> PathMap;
    typedef std::unordered_map<std::string, UIJointKey> OccupantMap;

    /* data */
    std::string const   name;
    UIJointMap const    joints;
    UIJointKeys const   leaf_joints;      // Leaf joints are tips of the path.
    OccupantMap const   occupant_map;  // Occupied joints are a subset of leaf joints.
    WorkType const      type;
    PathMap const       path_map;
    size_t const        path_len;
    size_t const        target_size;

    /* ctors */
    WorkArea(std::string const& _name,
             JSON const& json,
             FixedAreaMap const& fam);

    /* dtors */
    virtual ~WorkArea();

    static TestStat test();
};

typedef SPMap<WorkArea> WorkAreaMap;

}  /* elfin */

#endif  /* end of include guard: WORK_AREA_H_ */