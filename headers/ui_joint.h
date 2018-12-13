#ifndef UI_JOINT_H_
#define UI_JOINT_H_

#include <unordered_map>
#include <string>
#include <tuple>

#include "ui_object.h"
#include "string_utils.h"

namespace elfin {

class UIJoint : public UIObject
{
protected:
    StrList neighbours_;

public:
    std::tuple<std::string, std::string, UIObject const *> occupant_triple_ =
        std::make_tuple("", "", nullptr);
    std::tuple<std::string, UIJoint const *> hinge_tuple_ =
        std::make_tuple("", nullptr);

    UIJoint(JSON const& j, const std::string& name) :
        UIObject(j, name) {
        JSON const& jnbs = j["neighbours"];
        for (auto it = jnbs.begin(); it != jnbs.end(); ++it) {
            neighbours_.push_back(*it);
        }
    }

    /* getter */
    StrList const& neighbours() const {
        return neighbours_;
    }
};

typedef std::unordered_map<std::string, UIJoint> UIJointMap;

}  /* elfin */

#endif  /* end of include guard: UI_JOINT_H_ */