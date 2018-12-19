#include "work_area.h"

#include <tuple>
#include <sstream>

#include "debug_utils.h"
#include "input_manager.h"

namespace elfin {

/* free */
void bad_work_type(WorkType type) {
    NICE_PANIC(type == type, string_format("Bad WorkType: %s\n",
                                           WorkTypeToCStr(type)));
}

/* private */
struct WorkArea::PImpl {
    /* data */
    const std::string name;
    WorkType type = WorkType::FREE;
    UIJointMap joints;
    std::vector<UIJoint *> occupied_joints;
    std::vector<UIJoint *> hinged_joints;
    std::vector<UIJoint *> leaf_joints;
    size_t target_size = 0;

    PImpl() = delete;
    PImpl(std::string const& _name) : name(_name) {}

    /* accessors */
    WorkType _determine_type(size_t const n_branches) const {
        WorkType res = WorkType::NONE;
        if (n_branches == 0) {
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
        float sum_dist = 0.0f;
        V3fList const shape = to_points();
        for (auto i = shape.begin() + 1; i != shape.end(); ++i) {
            sum_dist += (i - 1)->dist_to(*i);
        }

        // Add one because division counts segments. We want number of points.
        size_t const expected_len =
            1 + round(sum_dist / OPTIONS.avg_pair_dist);

        size_t const res = expected_len + OPTIONS.len_dev_alw;

        return res;
    }

    V3fList to_points() const {
        V3fList res;
        NICE_PANIC(leaf_joints.size() != 2,
                   string_format("Size of leaf_joints not "
                                 "exactly 2 in work_area: %s\n",
                                 name.c_str()));

        UIJoint const* prev = nullptr;
        UIJoint const* j = leaf_joints.at(0);
        while (1) {
            res.emplace_back(j->tx().collapsed());

            if (j == leaf_joints.at(1)) {
                break;
            }

            std::string next_name = j->neighbours().at(0);

            NICE_PANIC(j->neighbours().size() > 2);
            if (prev and next_name == prev->name()) {
                next_name = j->neighbours().at(1);
            }

            prev = j;
            j = &joints.at(next_name);
        }

        return res;
    }

    /* modifiers */
    void reset() {
        type = WorkType::FREE;
        joints = UIJointMap();
        occupied_joints = std::vector<UIJoint *>();
        hinged_joints = std::vector<UIJoint *>();
        leaf_joints = std::vector<UIJoint *>();
        target_size = 0;
    }

    void parse_from_json(const JSON& j) {
        /*
         * Parse joints from JSON and collect special joints.
         */
        reset();

        size_t n_branches = 0;
        for (auto it = j.begin(); it != j.end(); ++it) {
            const JSON& jt_json = *it;
            const std::string& joint_name = it.key();
            auto j_itr = joints.emplace(joint_name, UIJoint(jt_json, joint_name));
            const size_t n_nbs = jt_json["neighbours"].size();
            n_branches += n_nbs > 2;

            auto& key_val = *j_itr.first;
            UIJoint* ptr = &(key_val.second);
            if (jt_json["occupant"] != "") {
                ptr->occupant_triple_ =
                    std::make_tuple(
                        jt_json["occupant_parent"],
                        jt_json["occupant"],
                        nullptr);
                occupied_joints.push_back(ptr);
            }

            if (jt_json["hinge"] != "") {
                ptr->hinge_tuple_ = std::make_tuple(jt_json["hinge"], nullptr);
                hinged_joints.push_back(ptr);
            }

            if (n_nbs == 1) {
                leaf_joints.push_back(ptr);
            }
        }

        type = _determine_type(n_branches);
        target_size = _determine_target_size();
    }
};

/* public */
/* ctors */
WorkArea::WorkArea(
    const JSON& j,
    const std::string& name) {
    p_impl_ = std::make_unique<PImpl>(name);
    p_impl_->parse_from_json(j);
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

std::vector<UIJoint *> const& WorkArea::occupied_joints() const {
    return p_impl_->occupied_joints;
}

std::vector<UIJoint *> const& WorkArea::hinged_joints() const {
    return p_impl_->hinged_joints;
}

V3fList WorkArea::to_points() const {
    return p_impl_->to_points();
}

}  /* elfin */