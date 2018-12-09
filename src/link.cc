#include "link.h"

#include "node.h"

namespace elfin {

/* accessors */
bool Link::operator==(const Link & other) const {
    return src_chain_ == other.src_chain_ and
           dst_chain_ == other.dst_chain_;
}

/* modifiers */
void Link::update_node_ptrs(const NodeAddrMap & nam) {
    src_chain_.node = nam.at(src_chain_.node);
    dst_chain_.node = nam.at(dst_chain_.node);
}

void Link::sever(const Link link) {
    link.src_chain_.node->remove_link(link);
    link.dst_chain_.node->remove_link(link.reversed());
}

}  /* elfin */