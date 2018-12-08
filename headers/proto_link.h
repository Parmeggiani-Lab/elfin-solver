#ifndef PROTO_LINK_H_
#define PROTO_LINK_H_

#include <vector>

#include "geometry.h"

namespace elfin {

class ProtoModule;

struct ProtoLink
{
    Transform tx;
    ProtoModule * target_mod;
    size_t target_chain_id;

    /* ctors */
    ProtoLink(const Transform & _tx, ProtoModule * _mod, const size_t chain_id) :
        tx(_tx), target_mod(_mod), target_chain_id(chain_id) {}

    /* dtors */
    virtual ~ProtoLink() {}

    /* operators  */
    bool operator<(const ProtoLink & rhs) const;
};

typedef std::vector<ProtoLink> ProtoLinkList;

}  /* elfin */

#endif  /* end of include guard: PROTO_LINK_H_ */