#include "link.h"

#include "node.h"
#include "pointer_utils.h"

namespace elfin {

/* public */
/* ctors */
Link::Link(
    FreeChain const& src_chain,
    ProtoLink const* prototype,
    FreeChain const& dst_chain) :
    src_chain_(src_chain),
    prototype_(prototype),
    dst_chain_(dst_chain) {
    DEBUG(dst_chain_.node_sp()->prototype_ != this->prototype_->module_);
}

/* accessors */
bool Link::operator==(Link const& other) const {
    return src_chain_ == other.src_chain_ and
           dst_chain_ == other.dst_chain_;
}

/* modifiers */
void Link::update_node_ptrs(NodeAddrMap const& nam) {
    src_chain_.node = nam.at(src_chain_.node_sp());
    DEBUG(is_uninitialized(src_chain_.node));
    dst_chain_.node = nam.at(dst_chain_.node_sp());
    DEBUG(is_uninitialized(dst_chain_.node));
}

/* printers */
std::string Link::to_string() const {
    return string_format("Link[\n  %s\n  %s\n]",
                         src_chain_.to_string().c_str(),
                         dst_chain_.to_string().c_str());
}

}  /* elfin */