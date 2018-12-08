#include "node.h"

#include <sstream>
#include <algorithm>

namespace elfin {

/* public */
Node::Node(
    const ProtoModule * prototype,
    const Transform & tx) :
    prototype_(prototype),
    tx_(tx) {
    // Reserve memory for maximum occupancy
    neighbors_.reserve(prototype_->counts().all_interfaces());
}

void Node::update_neighbor_ptrs(const NodeAddrMap & nam) {
    for (Link & link : neighbors_) {
        link.update_node_ptrs(nam);
    }
}

std::string Node::to_string() const {
    return string_format("Node[%s]\nTx: %s\n",
                         prototype_->name.c_str(),
                         tx_.to_string().c_str());
}

std::string Node::to_csv_string() const {
    UNIMPLEMENTED(); // This function probably need an update
    return tx_.to_csv_string();
}

}  /* elfin */