#include "ui_joint_path_generator.h"

namespace elfin {

/* public */
/* modifiers */
UIJointKey UIJointPathGenerator::next() {
    UIJointKey prev_joint = curr_joint_;
    curr_joint_ = next_joint_;
    next_joint_ = nullptr;

    // Look for next node
    if (curr_joint_) {
        size_t const num_neighbors = curr_joint_->neighbors.size();
        TRACE_NOMSG(num_neighbors > 2);
        for (auto& nb_name : curr_joint_->neighbors) {
            if (not prev_joint or nb_name != prev_joint->name) {
                next_joint_ = joints_->at(nb_name).get();
                break;
            }
        }
    }

    return curr_joint_;
}

}  /* elfin */