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
    UIJoint* curr_joint_ = nullptr;
    UIJoint* next_joint_ = nullptr;
public:
    /* ctors */
    BasicUIJointGenerator(
        UIJointMap const* const joints,
        UIJoint* const start_joint) :
        joints_(joints)
    {
        TRACE_PANIC(not start_joint);
        next_joint_ = start_joint;
    }

    /* dtors */
    virtual ~BasicUIJointGenerator() {}

    /* accessors */
    bool is_done() const { return not next_joint_; }

    UIJoint* curr_node() const { return curr_joint_; }

    /* modifiers */
    UIJoint* next();

};  /* class BasicUIJointGenerator */

}  /* elfin */

#endif  /* end of include guard: BASIC_UI_JOINT_GENERATOR_H_ */