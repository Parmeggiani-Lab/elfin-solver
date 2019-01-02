#ifndef UI_MODULE_H_
#define UI_MODULE_H_

#include "ui_object.h"
#include "map_utils.h"

namespace elfin {

struct UIModule : public UIObject {
    /* data */
    std::string const module_name;

    // Other fields:
    // module_type
    // c_linkage
    // n_linkage

    /* ctors */
    UIModule(std::string const& name,
             JSON const& json);

    /* dtors */
    virtual ~UIModule() {}

    /* printers */
    virtual void print_to(std::ostream& os) const;
};

typedef SPMap<UIModule> UIModuleMap;
typedef UIModule const* UIModuleKey;

}  /* elfin */

#endif  /* end of include guard: UI_MODULE_H_ */