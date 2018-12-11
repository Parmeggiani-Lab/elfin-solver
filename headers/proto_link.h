#ifndef PROTO_LINK_H_
#define PROTO_LINK_H_

#include <unordered_set>

#include "geometry.h"
#include "vector_utils.h"

namespace elfin {

class ProtoModule;
class ProtoLink;

class ProtoLink
{
private:
    Transform tx_;
    ProtoModule const* module_;
    size_t chain_id_;
    ProtoLink const* reverse_;

public:
    /* ctors */
    ProtoLink(
        Transform const& tx,
        ProtoModule const* module,
        size_t const chain_id);

    /* accessors */
    Transform const& tx() const { return tx_; }
    ProtoModule const* module() const { return module_; }
    size_t const& chain_id() const { return chain_id_; }
    ProtoLink const* reverse() const { return reverse_; }

    /* modifiers */
    static void pair_proto_links(
        ProtoLink & lhs,
        ProtoLink & rhs);
};

/* types */
typedef ProtoLink const* ConstProtoLinkPtr;

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
 * ProtoLink instance (by comparing ProtoLink * with FreeChain *).
 */
typedef std::unordered_set <
ConstProtoLinkPtr,
HashProtoLinkWithoutTx,
EqualProtoLinkWithoutTx>
ProtoLinkPtrSet;

typedef typename ProtoLinkPtrSet::const_iterator
ProtoLinkPtrSetCItr;

typedef Vector<ProtoLink> ProtoLinkList;

struct CompareProtoLinkByModuleInterfaces {
    bool operator() (
        ProtoLink const& lhs, ProtoLink const& rhs) const;
};

}  /* elfin */

#endif  /* end of include guard: PROTO_LINK_H_ */