#ifndef PROTO_LINK_H_
#define PROTO_LINK_H_

#include <unordered_set>
#include <memory>

#include "geometry.h"
#include "string_utils.h"
#include "proto_path.h"
#include "term_type.h"

namespace elfin {

/* Fwd Decl */
class ProtoModule;
typedef ProtoModule const* PtModKey;
struct ProtoLink;
typedef ProtoLink const* PtLinkKey;
typedef std::unique_ptr<ProtoLink> PtLinkSP;

struct ProtoLink : public Printable {
    /* data */
    Transform tx;
    PtModKey module;
    size_t chain_id;
    PtLinkKey reverse = nullptr;

    ProtoLink(Transform const& _tx,
              PtModKey const _module,
              size_t const _chain_id);

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