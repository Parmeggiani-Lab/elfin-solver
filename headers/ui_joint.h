#ifndef UI_JOINT_H_
#define UI_JOINT_H_

#include <unordered_map>
#include <string>
#include <tuple>

#include "ui_object.h"

namespace elfin {

class UIJoint : public UIObject
{
public:
    using UIObject::UIObject; // inherit ctors

    std::tuple<std::string, std::string, UIObject const *> occupant_triple_ =
        std::make_tuple("", "", nullptr);
    std::tuple<std::string, UIJoint const *> hinge_tuple_ =
        std::make_tuple("", nullptr);
    std::vector<UIJoint const *> neighbours_;
};

typedef std::unordered_map<std::string, UIJoint> UIJoints;

}  /* elfin */

#endif  /* end of include guard: UI_JOINT_H_ */