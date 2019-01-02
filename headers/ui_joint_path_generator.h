#ifndef UI_JOINT_PATH_GENERATOR_H_
#define UI_JOINT_PATH_GENERATOR_H_

#include <memory>

#include "ui_joint.h"
#include "debug_utils.h"

namespace elfin {

class UIJointPathGenerator {
private:
    /* data */
    UIJointMap const* joints_;
    UIJointKey curr_joint_ = nullptr;
    UIJointKey next_joint_ = nullptr;
public:
    /* ctors */
    UIJointPathGenerator(UIJointMap const* const joints,
                         UIJointKey const start_joint) :
        joints_(joints)
    {
        TRACE_NOMSG(not start_joint);
        next_joint_ = start_joint;
    }

    /* dtors */
    virtual ~UIJointPathGenerator() {}

    /* accessors */
    bool is_done() const { return not next_joint_; }

    UIJointKey curr_node() const { return curr_joint_; }

    /* modifiers */
    UIJointKey next();

};  /* class UIJointPathGenerator */

}  /* elfin */

#endif  /* end of include guard: UI_JOINT_PATH_GENERATOR_H_ */