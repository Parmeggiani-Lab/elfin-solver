#ifndef UI_MODULE_H_
#define UI_MODULE_H_

#include "ui_object.h"
#include "map_utils.h"
#include "term_type.h"
#include "free_term.h"

namespace elfin {

/* Fwd Decl */
class ProtoModule;
typedef ProtoModule const* PtModKey;
class ProtoTerm;
typedef ProtoTerm const* PtTermKey;

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
    PtModKey const prototype;
    Linkage const linkage;
    FreeTerms const free_terms;
    std::vector<PtTermKey> const free_ptterms;
    std::vector<PtModKey> provis_hubs;

    /* ctors */
    UIModule(std::string const& name,
             JSON const& json);

    // Provisional UIModule
    UIModule(std::string const& _name,
             Vector3f const& _pos);

    /* dtors */
    virtual ~UIModule() {}

    /* printers */
    virtual void print_to(std::ostream& os) const;
};

typedef SPMap<UIModule> UIModuleMap;
typedef UIModule const* UIModKey;

}  /* elfin */

#endif  /* end of include guard: UI_MODULE_H_ */