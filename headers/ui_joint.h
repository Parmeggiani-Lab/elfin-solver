#ifndef UI_JOINT_H_
#define UI_JOINT_H_

#include <string>
#include <tuple>

#include "ui_object.h"
#include "map_utils.h"
#include "fixed_area.h"

namespace elfin {

class UIJoint : public UIObject {
protected:;
    /* types */
    struct Occupant {
        std::string parent_name = "";
        std::string name = "";
        std::shared_ptr<UIObject> module = nullptr;
    };

    /* data */
    StrList neighbors_;
    Occupant occupant_;
    std::string hinge_name_ = "";

public:
    /* ctors */
    UIJoint(
        JSON const& json,
        std::string const& name,
        FixedAreaMap const& fam);

    /* accessors */
    StrList const& neighbors() const { return neighbors_; }
    Occupant const& occupant() const { return occupant_; }
};

typedef std::shared_ptr<UIJoint> UIJointSP;
typedef SPMap<UIJoint> UIJointMap;

}  /* elfin */

#endif  /* end of include guard: UI_JOINT_H_ */