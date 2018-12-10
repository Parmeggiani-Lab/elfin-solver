#ifndef PROTO_TERMINUS_H_
#define PROTO_TERMINUS_H_

#include <unordered_set>

#include "proto_link.h"
#include "terminus_type.h"
#include "roulette.h"

namespace elfin {

class ProtoModule;

/* types */
typedef const ProtoLink * ConstProtoLinkPtr;
typedef const ProtoModule * ConstProtoModulePtr;
struct ProtoLinkHashWithoutTx {
    using is_transparent = void;
    size_t operator()(const ConstProtoLinkPtr & link) const {
        return std::hash<void *>()((void *) link->target_mod) ^
               std::hash<size_t>()(link->target_chain_id);
    }
};
struct ProtoLinkEqualWithoutTx {
    using is_transparent = void;
    bool operator()(
        const ConstProtoLinkPtr & lh_link,
        const ConstProtoLinkPtr & rh_link) const {
        return lh_link->target_mod == rh_link->target_mod and
               lh_link->target_chain_id == rh_link->target_chain_id;
    }
};
typedef std::unordered_set <
ConstProtoLinkPtr,
ProtoLinkHashWithoutTx,
ProtoLinkEqualWithoutTx > ProtoLinkPtrSet;
typedef typename ProtoLinkPtrSet::const_iterator ProtoLinkPtrSetCItr;

class ProtoTerminus {
    friend ProtoModule;
private:
    /* data */
    bool finalized_ = false;
    ProtoLinkList proto_link_list_;
    Roulette<ProtoLink *> n_roulette_, c_roulette_;
    ProtoLinkPtrSet proto_link_set_;

public:
    /* accessors */
    const ProtoLinkList & proto_links() const { return proto_link_list_; }
    const ProtoLink & pick_random_proto_link(const TerminusType term) const;
    const ProtoLinkPtrSet & proto_link_set() const { return proto_link_set_; }
    ProtoLinkPtrSetCItr find_link_to(
        ConstProtoModulePtr module,
        const size_t chain_id) const;
    bool has_link_to(
        ConstProtoModulePtr module,
        const size_t chain_id) const {
        return find_link_to(module, chain_id) != proto_link_set_.end();
    }

    /* modifiers */
    void finalize();
};

}  /* elfin */

#endif  /* end of include guard: PROTO_TERMINUS_H_ */