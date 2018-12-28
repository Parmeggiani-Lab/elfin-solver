#ifndef BASIC_UI_JOINT_GENERATOR_H_
#define BASIC_UI_JOINT_GENERATOR_H_

#include <memory>

#include "ui_joint.h"
#include "debug_utils.h"

namespace elfin {

class BasicUIJointGenerator {
private:
    /* data */
    UIJointMap const* joints_;
    UIJointSP curr_joint_ = nullptr;
    UIJointSP next_joint_ = nullptr;
public:
    /* ctors */
    BasicUIJointGenerator(
        UIJointMap const* joints,
        UIJointSP start_joint) :
        joints_(joints),
        next_joint_(start_joint) {
        TRACE_PANIC(not start_joint);
    }

    /* dtors */
    virtual ~BasicUIJointGenerator() {}

    /* accessors */
    bool is_done() const { return next_joint_ == nullptr; }

    UIJointSP curr_node() const { return curr_joint_; }

    /* modifiers */
    UIJointSP next();

};  /* class BasicUIJointGenerator */

}  /* elfin */

#endif  /* end of include guard: BASIC_UI_JOINT_GENERATOR_H_ */