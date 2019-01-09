#ifndef PROTO_LINK_H_
#define PROTO_LINK_H_

#include <unordered_set>
#include <memory>

#include "geometry.h"
#include "string_utils.h"
#include "term_type.h"

namespace elfin {

/* Fwd Decl */
class ProtoModule;
typedef ProtoModule const* PtModKey;
struct ProtoLink;
typedef ProtoLink const* PtLinkKey;
typedef std::unique_ptr<ProtoLink> PtLinkSP;
class ProtoTerm;

struct ProtoLink : public Printable {
    /* data */
    Transform tx;
    PtModKey module;  // Dst module.
    size_t chain_id;  // For dst module.
    TermType term;    // For dst module.
    PtLinkKey reverse = nullptr;

    /* ctors */
    ProtoLink(Transform const& _tx,
              PtModKey const _module,
              size_t const _chain_id,
              TermType const _term) :
        tx(_tx),
        module(_module),
        chain_id(_chain_id),
        term(_term) {}

    /* accessors */
    ProtoTerm const& get_term() const;

    /* printers */
    virtual void print_to(std::ostream& os) const;
};

/* types */
typedef std::vector<PtLinkSP> PtLinks;

struct HashPtLinkSimple {
    size_t operator()(PtLinkKey const& link) const;
};

struct EqualPtLinkSimple {
    bool operator()(PtLinkKey const& lh_link,
                    PtLinkKey const& rh_link) const;
};

}  /* elfin */

#endif  /* end of include guard: PROTO_LINK_H_ */