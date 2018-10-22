#ifndef UI_JOINT_H_
#define UI_JOINT_H_

#include <vector>

#include "ui_object.h"

namespace elfin {

class UIJoint : public UIObject
{
public:
    using UIObject::UIObject; // inherit ctors

    UIObject const * occupant_;
    UIJoint const * hinge_;
    std::vector<UIJoint const *> neighbours_;
};

typedef std::vector<UIJoint> UIJoints;

}  /* elfin */

#endif  /* end of include guard: UI_JOINT_H_ */