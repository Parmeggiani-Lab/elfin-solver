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
    /* data */
    std::string name;
    WorkType type = WorkType::FREE;
    UIJointMap joints;
    std::vector<UIJoint*> occupied_joints;
    std::vector<UIJoint*> leaf_joints;
    size_t target_size = 0;
    V3fList points;

    /* ctors */
    // Parse joints from JSON and collect special joints.
    PImpl(
        std::string const& _name,
        JSON const& json,
        FixedAreaMap const& fam) :
        name(_name) {
        size_t num_branch_points = 0;
        for (auto it = begin(json); it != end(json); ++it) {
            JSON const& joint_json = *it;
            std::string const& joint_name = it.key();

            // Create new joint and put it in map
            joints.emplace(
                joint_name,
                std::make_unique<UIJoint>(joint_json, joint_name, fam));
            auto& joint = joints.at(joint_name);

            // Count branch points
            size_t const num_neighbors = joint->neighbors().size();

            num_branch_points += num_neighbors > 2;
            if (num_neighbors == 1) {
                leaf_joints.push_back(joint.get());
            }

            if (joint->occupant().name != "") {
                occupied_joints.push_back(joint.get());
            }
        }

        type = _determine_type(num_branch_points);
        points = _gen_points(); // Cache it
        target_size = _determine_target_size();
    }

    /* accessors */
    WorkType _determine_type(
        size_t const num_branch_points) const {
        WorkType res = WorkType::NONE;
        if (num_branch_points == 0) {
            switch (occupied_joints.size()) {
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

    size_t _determine_target_size() const {
        /*
         * Calculate expected length as sum of point
         * displacements over avg pair module distance
         */
        TRACE_NOMSG(points.size() == 0);

        float sum_dist = 0.0f;
        for (auto i = begin(points) + 1; i != end(points); ++i) {
            sum_dist += (i - 1)->dist_to(*i);
        }

        // Add one because division counts segments. We want number of points.
        size_t const expected_len =
            1 + round(sum_dist / OPTIONS.avg_pair_dist);

        size_t const res = expected_len + OPTIONS.len_dev;

        return res;
    }

    V3fList _gen_points() const {
        V3fList res;
        TRACE(leaf_joints.size() != 2,
              "Size of leaf_joints not exactly 2 in work_area: %s\n",
              name.c_str());
        UIJointPathGenerator gen(&joints, leaf_joints.at(0));

        while (not gen.is_done()) {
            res.emplace_back(gen.next()->tx.collapsed());
        }

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