#include "proto_link.h"

#include "proto_module.h"

namespace elfin {

ProtoLink::ProtoLink(Transform const& tx,
                     ProtoModule const* module,
                     size_t const chain_id) :
    tx_(tx),
    module_(module),
    chain_id_(chain_id)
{}

/* modifiers */
// static
void ProtoLink::pair_links(
    ProtoLink* lhs, ProtoLink* rhs) {
    TRACE(not lhs->tx_.is_approx(rhs->tx_.inversed()),
          "lhs->tx=%s\nrhs->tx.inversed()=%s\n",
          lhs->tx_.to_string().c_str(),
          rhs->tx_.inversed().to_string().c_str());
    lhs->reverse_ = rhs;
    rhs->reverse_ = lhs;
}

/* printers */
void ProtoLink::print_to(std::ostream& os) const {
    os << "ProtoLink (" << module_->name << ") [\n";
    os << "  Chain ID: " << chain_id_ << "\n";
    os << tx_ << "\n";
    os << "]";
}

size_t HashPtLinkWithoutTx::operator()(
    PtLinkKey const& link) const {
    return std::hash<void *>()((void *) link->module_) ^
           std::hash<size_t>()(link->chain_id_);
}

bool EqualPtLinkWithoutTx::operator()(
    PtLinkKey const& lh_link,
    PtLinkKey const& rh_link) const {
    return lh_link->module_ == rh_link->module_ and
           lh_link->chain_id_ == rh_link->chain_id_;
}


}  /* elfin */