#include "proto_link.h"

#include "proto_module.h"

namespace elfin {

/* operators */
bool ProtoLink::operator<(const ProtoLink & rhs) const {
    return target_mod->counts().all_interfaces() <
           rhs.target_mod->counts().all_interfaces();
}

}  /* elfin */