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
    /* types */
    typedef Roulette<ProtoLink const*> ProtoLinkRoulette; 
    /* data */
    bool finalized_ = false;
    ProtoLinkPtrList links_;
    ProtoLinkRoulette n_roulette_, c_roulette_;
    ProtoLinkPtrSet link_set_;

public:
    /* ctors */
    ProtoTerminus() {}
    ProtoTerminus(ProtoTerminus const& other) = delete;
    ProtoTerminus(ProtoTerminus && other) = delete;
    ProtoTerminus& operator=(ProtoTerminus const& other) = delete;
    ProtoTerminus& operator=(ProtoTerminus && other) = delete;

    /* dtors */
    virtual ~ProtoTerminus();

    /* accessors */
    ProtoLinkPtrList const& links() const { return links_; }
    ProtoLink const& pick_random_link(TerminusType const term) const;
    ProtoLinkPtrSet const& link_set() const { return link_set_; }
    ProtoLinkPtrSetCItr find_link_to(
        ConstProtoModulePtr dst_module,
        size_t const dst_chain_id) const;
    bool has_link_to(
        ConstProtoModulePtr dst_module,
        size_t const dst_chain_id) const {
        return find_link_to(dst_module, dst_chain_id) != link_set_.end();
    }

    /* modifiers */
    void finalize();
};

}  /* elfin */

#endif  /* end of include guard: PROTO_TERMINUS_H_ */