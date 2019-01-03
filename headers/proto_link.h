#ifndef PROTO_LINK_H_
#define PROTO_LINK_H_

#include <unordered_set>
#include <memory>

#include "geometry.h"
#include "string_utils.h"

namespace elfin {

class ProtoModule;

class ProtoLink : public Printable {
private:
    ProtoLink const* reverse_;

public:
    /* data */
    Transform const tx_;
    ProtoModule const* const module_;
    size_t const chain_id_;

    /* ctors */
    ProtoLink(Transform const& tx,
              ProtoModule const* module,
              size_t const chain_id);

    /* accessors */
    ProtoLink const* reverse() const { return reverse_; }

    /* modifiers */
    static void pair_links(ProtoLink* lhs, ProtoLink* rhs);

    /* printers */
    virtual void print_to(std::ostream& os) const;
};

/* types */
typedef ProtoLink const* PtLinkKey;
typedef std::unique_ptr<ProtoLink> PtLinkSP;
typedef std::vector<PtLinkSP> PtLinkSPList;

struct HashPtLinkWithoutTx {
    size_t operator()(PtLinkKey const& link) const;
};

struct EqualPtLinkWithoutTx {
    bool operator()(PtLinkKey const& lh_link,
                    PtLinkKey const& rh_link) const;
};

}  /* elfin */

#endif  /* end of include guard: PROTO_LINK_H_ */