#ifndef UI_MODULE_H_
#define UI_MODULE_H_

#include "ui_object.h"
#include "map_utils.h"
#include "term_type.h"

namespace elfin {

struct UIModule : public UIObject {
    /* type */
    struct UILink {
        TermType const term;
        std::string const src_chain_name;
        std::string const dst_chain_name;
        std::string const target_ui_name;
    };
    typedef std::vector<UILink> Linkage;

    /* data */
    std::string const module_name;
    std::string const module_type;
    Linkage const linkage;

    /* ctors */
    UIModule(std::string const& name,
             JSON const& json);

    /* dtors */
    virtual ~UIModule() {}

    /* printers */
    virtual void print_to(std::ostream& os) const;
};

typedef SPMap<UIModule> UIModuleMap;
typedef UIModule const* UIModKey;

}  /* elfin */

#endif  /* end of include guard: UI_MODULE_H_ */