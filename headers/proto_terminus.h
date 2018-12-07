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
    ProtoLinkList proto_links_;
    Roulette<ProtoLink *> n_rlt_;
    Roulette<ProtoLink *> c_rlt_;

public:
    /* data */
    const ProtoLinkList & proto_links() const { return proto_links_; }

    /* accessors */
    const ProtoLink & pick_random_proto_link(const TerminusType term) const;

    /* modifiers */
    void finalize();
};

}  /* elfin */

#endif  /* end of include guard: PROTO_TERMINUS_H_ */