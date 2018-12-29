#include "link.h"

#include "node.h"
#include "path_generator.h"

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
    DEBUG(dst_chain_.node->prototype_ != this->prototype_->module_);
}

/* accessors */
bool Link::operator==(Link const& other) const {
    return src_chain_ == other.src_chain_ and
           dst_chain_ == other.dst_chain_;
}

PathGenerator Link::gen_path() const {
    return PathGenerator(this);
}

/* modifiers */
void Link::update_node_ptrs(NodeAddrMap const& nam) {
    src_chain_.node = nam.at(src_chain_.node);
    DEBUG(not src_chain_.node);
    dst_chain_.node = nam.at(dst_chain_.node);
    DEBUG(not dst_chain_.node);
}

/* printers */
void Link::print_to(std::ostream& os) const {
    os << "Link[\n  " << src_chain_;
    os << "\n  " << dst_chain_ << "\n]";
}

}  /* elfin */