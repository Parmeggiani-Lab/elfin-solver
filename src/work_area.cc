#include "work_area.h"

#include <tuple>
#include <sstream>

#include "debug_utils.h"
#include "input_manager.h"
#include "fixed_area.h"
#include "basic_ui_joint_generator.h"

namespace elfin {

/* free */
void bad_work_type(WorkType type) {
    NICE_PANIC(type == type, string_format("Bad WorkType: %s\n",
                                           WorkTypeToCStr(type)));
}

/* private */
struct WorkArea::PImpl {
    /* types */
    typedef std::vector<std::shared_ptr<UIJoint>> UIJointSPList;

    /* data */
    std::string const name;
    WorkType type = WorkType::FREE;
    UIJointMap joints;
    UIJointSPList occupied_joints;
    UIJointSPList leaf_joints;
    size_t target_size = 0;
    V3fList points;

    PImpl() = delete;
    PImpl(std::string const& _name) : name(_name) {}

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
        NICE_PANIC(points.size() == 0);

        float sum_dist = 0.0f;
        for (auto i = points.begin() + 1; i != points.end(); ++i) {
            sum_dist += (i - 1)->dist_to(*i);
        }

        // Add one because division counts segments. We want number of points.
        size_t const expected_len =
            1 + round(sum_dist / OPTIONS.avg_pair_dist);

        size_t const res = expected_len + OPTIONS.len_dev_alw;

        return res;
    }

    V3fList _to_points() const {
        V3fList res;
        NICE_PANIC(leaf_joints.size() != 2,
                   string_format("Size of leaf_joints not "
                                 "exactly 2 in work_area: %s\n",
                                 name.c_str()));
        BasicUIJointGenerator gen(&joints, leaf_joints.at(0));

        while (not gen.is_done()) {
            UIJointSP curr_joint = gen.next();
            res.emplace_back(curr_joint->tx.collapsed());
        }

        return res;
    }

    /* modifiers */
    void reset() {
        type = WorkType::FREE;
        joints.clear();
        occupied_joints.clear();
        leaf_joints.clear();
        target_size = 0;
        points.clear();
    }

    void parse_from_json(
        JSON const& json,
        FixedAreaMap const& fam) {
        /*
         * Parse joints from JSON and collect special joints.
         */
        reset();

        size_t num_branch_points = 0;
        for (auto it = json.begin(); it != json.end(); ++it) {
            JSON const& joint_json = *it;
            std::string const& joint_name = it.key();

            // Create new joint and put it in map
            joints.emplace(
                joint_name,
                std::make_shared<UIJoint>(joint_json, joint_name, fam));
            auto& joint = joints.at(joint_name); // shared_ptr

            // Count branch points
            size_t const num_neighbors = joint->neighbors().size();
            NICE_PANIC(num_neighbors != joint_json["neighbors"].size());

            num_branch_points += num_neighbors > 2;
            if (num_neighbors == 1) {
                leaf_joints.push_back(joint);
            }

            if (joint->occupant().name != "") {
                occupied_joints.push_back(joint);
            }
        }

        type = _determine_type(num_branch_points);
        points = _to_points(); // Cache it
        target_size = _determine_target_size();
    }
};

/* public */
/* ctors */
WorkArea::WorkArea(
    JSON const& j,
    std::string const& name,
    FixedAreaMap const& fam) {
    p_impl_ = std::make_unique<PImpl>(name);
    p_impl_->parse_from_json(j, fam);
}

/* dtors */
WorkArea::~WorkArea() {}

/* accessors */

std::string WorkArea::name() const {
    return p_impl_->name;
}

WorkType WorkArea::type() const {
    return p_impl_->type;
}

UIJointMap const& WorkArea::joints() const {
    return p_impl_->joints;
}

size_t WorkArea::target_size() const {
    return p_impl_->target_size;
}

V3fList WorkArea::to_points() const {
    return p_impl_->points;
}

}  /* elfin */