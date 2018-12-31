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

/* private */
struct WorkArea::PImpl {
    /* types */
    typedef std::vector<UIJoint*> UIJointList;
    /* data */
    std::string const   name;
    UIJointMap const    joints;
    UIJointList const   occupied_joints;
    UIJointList const   leaf_joints;
    WorkType const      type;
    V3fList const       points;
    size_t const        target_size;

    /* ctors */
    PImpl(
        std::string const& _name,
        JSON const& json,
        FixedAreaMap const& fam) :
        /* Be careful with member ordering! */
        name(_name),
        joints(parse_joints(json, fam)),
        occupied_joints(parse_occupied_joints(joints)),
        leaf_joints(parse_leaf_joints(joints)),
        type(parse_type(joints, occupied_joints)),
        points(parse_points(joints, leaf_joints)),
        target_size(parse_target_size(points))
    {}

    /* data initializers */
    static UIJointMap parse_joints(JSON const& json,
                                   FixedAreaMap const& fam)
    {
        UIJointMap res;
        for (auto& [joint_name, joint_json] : json.items()) {
            res.emplace(
                joint_name,
                std::make_unique<UIJoint>(joint_json, joint_name, fam));
        }
        return res;
    }

    static UIJointList parse_occupied_joints(UIJointMap const& _joints)
    {
        UIJointList res;
        for (auto& [name, joint] : _joints) {
            if (joint->occupant().name != "") {
                res.push_back(joint.get());
            }
        }
        return res;
    }

    static UIJointList parse_leaf_joints(UIJointMap const& _joints)
    {
        UIJointList res;
        for (auto&  [name, joint] : _joints) {
            if (joint->neighbors().size() == 1) {
                res.push_back(joint.get());
            }
        }
        return res;
    }

    static WorkType parse_type(UIJointMap const& _joints,
                               UIJointList const& _occupied_joints)
    {
        size_t const num_branch_points =
            std::accumulate(
                begin(_joints),
                end(_joints),
                0,
        [](size_t sum, auto & map_itr) {
            return sum += map_itr.second->neighbors().size() > 2;
        });

        WorkType res = WorkType::NONE;
        if (num_branch_points == 0) {
            switch (_occupied_joints.size()) {
            case 0: {
                res = WorkType::FREE;
                break;
            }
            case 1: {
                res = WorkType::ONE_HINGE;
                break;
            }
            case 2: {
                res = WorkType::TWO_HINGE;
                break;
            }
            default:
            {
                res = WorkType::COMPLEX;
            }
            }
        } else {
            res = WorkType::COMPLEX;
        }

        return res;
    }

    static V3fList parse_points(UIJointMap const& _joints,
                                UIJointList const& _leaf_joints)
    {
        V3fList res;
        TRACE(_leaf_joints.size() != 2,
              "Size of _leaf_joints not exactly 2 in work_area\n");

        UIJointPathGenerator gen(&_joints, _leaf_joints.at(0));
        while (not gen.is_done()) {
            res.emplace_back(gen.next()->tx.collapsed());
        }

        return res;
    }

    static size_t parse_target_size(V3fList const& _points)
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
};

/* public */
/* ctors */
WorkArea::WorkArea(
    std::string const& name,
    JSON const& json,
    FixedAreaMap const& fam) {
    pimpl_ = std::make_unique<PImpl>(name, json, fam);
}

/* dtors */
WorkArea::~WorkArea() {}

/* accessors */
std::string WorkArea::name() const {
    return pimpl_->name;
}

WorkType WorkArea::type() const {
    return pimpl_->type;
}

UIJointMap const& WorkArea::joints() const {
    return pimpl_->joints;
}

size_t WorkArea::target_size() const {
    return pimpl_->target_size;
}

V3fList const& WorkArea::points() const {
    return pimpl_->points;
}

}  /* elfin */