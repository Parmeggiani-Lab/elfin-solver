#include "link.h"

namespace elfin {

bool Link::operator==(const Link & other) const {
    return src_chain_ == other.src_chain_ and
           dst_chain_ == other.dst_chain_;
}


void Link::update_node_ptrs(const NodeAddrMap & nam) {
    src_chain_.node = nam.at(src_chain_.node);
    dst_chain_.node = nam.at(dst_chain_.node);
}

}  /* elfin */