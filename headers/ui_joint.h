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
    std::tuple<std::string, std::string, UIObject const *> occupant_triple_ =
        std::make_tuple("", "", nullptr);
    std::tuple<std::string, UIJoint const *> hinge_tuple_ =
        std::make_tuple("", nullptr);
    std::vector<std::string> neighbours_;

    UIJoint(const JSON & j, const std::string & name) :
        UIObject(j, name) {
        const JSON & jnbs = j["neighbours"];
        for (auto it = jnbs.begin(); it != jnbs.end(); ++it) {
            neighbours_.push_back(*it);
        }
    }
};

typedef std::unordered_map<std::string, UIJoint> UIJointMap;

}  /* elfin */

#endif  /* end of include guard: UI_JOINT_H_ */