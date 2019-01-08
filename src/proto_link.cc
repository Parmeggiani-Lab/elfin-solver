#include "proto_link.h"

#include "proto_module.h"

namespace elfin {

/* ctors */
ProtoLink::ProtoLink(Transform const& _tx,
                     PtModKey const _module,
                     size_t const _chain_id,
                     PtLinkKey const _reverse) :
    tx(_tx),
    module(_module),
    chain_id(_chain_id),
    reverse(_reverse) {}

/* printers */
void ProtoLink::print_to(std::ostream& os) const {
    os << "ProtoLink (" << module->name << ") [\n";
    os << "  Chain ID: " << chain_id << "\n";
    os << tx << "\n";
    os << "]";
}

size_t HashPtLinkSimple::operator()(PtLinkKey const& link) const {
    return std::hash<void *>()((void *) link->module) ^
           std::hash<size_t>()(link->chain_id);
}

bool EqualPtLinkSimple::operator()(
    PtLinkKey const& lh_link,
    PtLinkKey const& rh_link) const
{
    return lh_link->module == rh_link->module and
           lh_link->chain_id == rh_link->chain_id;
}


}  /* elfin */