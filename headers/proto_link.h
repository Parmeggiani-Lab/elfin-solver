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
typedef ProtoLink const* ConstProtoLinkPtr;
typedef std::unique_ptr<ProtoLink> ProtoLinkSP;
typedef std::vector<ProtoLinkSP> ProtoLinkSPList;
typedef ProtoModule const* ConstProtoModulePtr;

struct HashProtoLinkWithoutTx {
    size_t operator()(ConstProtoLinkPtr const& link) const;
};

struct EqualProtoLinkWithoutTx {
    bool operator()(
        ConstProtoLinkPtr const& lh_link,
        ConstProtoLinkPtr const& rh_link) const;
};

/*
 * Note: with c++20, std::unordered_set::find(K& key) becomes a template
 * method. We'll be able to search the pointer set without creating a new
 * ProtoLink instance (by comparing ProtoLink* with FreeTerm *).
 */
typedef std::unordered_set <
ConstProtoLinkPtr,
HashProtoLinkWithoutTx,
EqualProtoLinkWithoutTx >
ProtoLinkPtrSet;

typedef typename ProtoLinkPtrSet::const_iterator
ProtoLinkPtrSetCItr;

typedef std::vector<ConstProtoLinkPtr> ProtoLinkPtrList;

}  /* elfin */

#endif  /* end of include guard: PROTO_LINK_H_ */