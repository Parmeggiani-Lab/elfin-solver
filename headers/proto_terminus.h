#ifndef PROTO_TERMINUS_H_
#define PROTO_TERMINUS_H_

#include "proto_link.h"
#include "terminus_type.h"
#include "roulette.h"

namespace elfin {

class ProtoModule;

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
        ConstProtoModulePtr dst_module,
        const size_t dst_chain_id) const;
    bool has_link_to(
        ConstProtoModulePtr dst_module,
        const size_t dst_chain_id) const {
        return find_link_to(dst_module, dst_chain_id) != proto_link_set_.end();
    }

    /* modifiers */
    void finalize();
};

}  /* elfin */

#endif  /* end of include guard: PROTO_TERMINUS_H_ */