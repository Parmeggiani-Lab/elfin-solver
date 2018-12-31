#include "work_area.h"

#include <tuple>
#include <sstream>

#include "debug_utils.h"
#include "input_manager.h"
#include "fixed_area.h"
#include "ui_joint_path_generator.h"

namespace elfin {

/* free */
void bad_work_type(WorkType type) {
    TRACE(WorkTypeToCStr(type), "Bad WorkType: %s\n", WorkTypeToCStr(type));
}

/* data initializers */
UIJointMap parse_joints(JSON const& json,
                        FixedAreaMap const& fam)
{
    UIJointMap res;
    for (auto& [joint_name, joint_json] : json.items()) {
        res.emplace(
            joint_name,
            std::make_unique<UIJoint>(joint_name, joint_json, fam));
    }
    return res;
}

UIJointKeys parse_leaf_joints(UIJointMap const& _joints)
{
    UIJointKeys res;
    for (auto& [name, joint] : _joints) {
        if (joint->neighbors.size() == 1) {
            res.push_back(joint.get());
        }
    }
    return res;
}

// Occupied joints are supposed to be a subset of leaves.
UIJointKeys parse_occupied_joints(UIJointKeys const& _leaf_joints)
{
    UIJointKeys res;
    for (auto& leaf_joint : _leaf_joints) {
        if (leaf_joint->occupant.ui_module) {
            res.push_back(leaf_joint);
        }
    }
    return res;
}

WorkType parse_type(UIJointMap const& _joints,
                    UIJointKeys const& _occupied_joints)
{
    WorkType res = WorkType::NONE;

    size_t const n_occ_joints = _occupied_joints.size();
    TRACE(n_occ_joints > 2,
          "Parsing error: too many occupied joints (%zu) for WorkArea\n",
          n_occ_joints);

    switch (n_occ_joints) {
    case 0:
        res = WorkType::FREE;
        break;
    case 1:
        res = WorkType::HINGED;
        break;
    case 2:
        res = WorkType::DOUBLE_HINGED;
        break;
    default:
        TRACE("Too many occupied joints",
              "Expecting 0, 1, or 2 but got %zu\n",
              n_occ_joints);
    }

    return res;
}

V3fList parse_points(UIJointMap const& _joints,
                     UIJointKeys const& _leaf_joints,
                     UIJointKeys const& _occupied_joints)
{
    V3fList res;
    TRACE(_leaf_joints.size() != 2,
          "Size of _leaf_joints not exactly 2 in work_area\n");

    // Start at the first occupied joint if there are any.
    auto start_joint_key = _occupied_joints.empty() ?
                           _leaf_joints.at(0) :
                           _occupied_joints.at(0);

    UIJointPathGenerator gen(&_joints, start_joint_key);
    while (not gen.is_done()) {
        res.emplace_back(gen.next()->tx.collapsed());
    }

    return res;
}

size_t parse_target_size(V3fList const& _points)
{
    /*
     * Calculate expected length as sum of point
     * displacements over avg pair module distance
     */
    TRACE_NOMSG(_points.size() == 0);

    float sum_dist = 0.0f;
    for (auto i = begin(_points) + 1; i != end(_points); ++i) {
        sum_dist += (i - 1)->dist_to(*i);
    }

    // Add one to nubmer of segments.
    size_t const expected_len =
        1 + round(sum_dist / OPTIONS.avg_pair_dist);

    size_t const res = expected_len + OPTIONS.len_dev;

    return res;
}

/* public */
/* ctors */
WorkArea::WorkArea(
    std::string const& _name,
    JSON const& json,
    FixedAreaMap const& fam):
    /* Be careful with member ordering! */
    name(_name),
    joints(parse_joints(json, fam)),
    leaf_joints(parse_leaf_joints(joints)),
    occupied_joints(parse_occupied_joints(leaf_joints)),
    type(parse_type(joints, occupied_joints)),
    points(parse_points(joints, leaf_joints, occupied_joints)),
    target_size(parse_target_size(points))
{ }

/* dtors */
WorkArea::~WorkArea() {}

}  /* elfin */