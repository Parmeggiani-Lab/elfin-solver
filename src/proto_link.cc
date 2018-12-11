#include "proto_link.h"

#include "proto_module.h"

namespace elfin {

ProtoLink::ProtoLink(
    Transform const& tx,
    ProtoModule const* module,
    size_t const chain_id) :
    tx_(tx),
    module_(module),
    chain_id_(chain_id)
{}

/* modifiers */
// static
void ProtoLink::pair_proto_links(
    ProtoLink & lhs,
    ProtoLink & rhs) {
    NICE_PANIC(lhs.tx_ != rhs.tx_.inversed(),
               string_format(
                   "lhs.tx=%s\nrhs.tx.inversed()=%s\n",
                   lhs.tx_.to_string().c_str(),
                   rhs.tx_.inversed().to_string().c_str()));
    lhs.reverse_ = &rhs;
    rhs.reverse_ = &lhs;
}

size_t HashProtoLinkWithoutTx::operator()(
    ConstProtoLinkPtr const& link) const {
    return std::hash<void *>()((void *) link->module()) ^
           std::hash<size_t>()(link->chain_id());
}

bool EqualProtoLinkWithoutTx::operator()(
    ConstProtoLinkPtr const& lh_link,
    ConstProtoLinkPtr const& rh_link) const {
    return lh_link->module() == rh_link->module() and
           lh_link->chain_id() == rh_link->chain_id();
}

bool CompareProtoLinkByModuleInterfaces::operator() (
    ProtoLink const& lhs,
    ProtoLink const& rhs) const {
    return lhs.module()->counts().all_interfaces() <
           rhs.module()->counts().all_interfaces();
}

}  /* elfin */