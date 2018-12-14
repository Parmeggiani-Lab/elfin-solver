#ifndef UI_JOINT_H_
#define UI_JOINT_H_

#include <unordered_map>
#include <string>
#include <tuple>

#include "ui_object.h"
#include "debug_utils.h"

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

    UIJoint(JSON const& j, std::string const& name) :
        UIObject(j, name) {
        try {
            JSON const& jnbs = j["neighbours"];
            for (auto it = jnbs.begin(); it != jnbs.end(); ++it) {
                neighbours_.push_back(*it);
            }
        } catch (const std::exception& e) {
            NICE_PANIC("Exception",
                       string_format("Failed to parse spec from JSON."
                                     "\nReason: %s", e.what()));
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