#ifndef UI_JOINT_H_
#define UI_JOINT_H_

#include <string>
#include <tuple>

#include "ui_object.h"
#include "map_utils.h"
#include "fixed_area.h"

namespace elfin {

struct UIJoint : public UIObject {
    /* types */
    struct Occupant : public Printable {
        /* data */
        std::string parent_name = "";
        UIModKey ui_module = nullptr;

        /* dtors */
        virtual ~Occupant() {}

        /* printers */
        virtual void print_to(std::ostream& os) const;
    };

    /* data */
    StrList const neighbors;
    Occupant const occupant;
    std::string const hinge_name;

    /* ctors */
    UIJoint(std::string const& name,
            JSON const& json,
            FixedAreaMap const& fam);

    /* dtors */
    virtual ~UIJoint() {};

    /* printers */
    virtual void print_to(std::ostream& os) const;
};

typedef std::unique_ptr<UIJoint> UIJointSP;
typedef SPMap<UIJoint> UIJointMap;
typedef UIJoint const* UIJointKey;
typedef std::vector<UIJointKey> UIJointKeys;

}  /* elfin */

#endif  /* end of include guard: UI_JOINT_H_ */