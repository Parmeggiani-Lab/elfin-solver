#include "proto_link.h"

#include "proto_module.h"

namespace elfin {

bool ProtoLink::operator<(const ProtoLink & rhs) const {
    return mod->counts().all_interfaces() < rhs.mod->counts().all_interfaces();
}

}  /* elfin */