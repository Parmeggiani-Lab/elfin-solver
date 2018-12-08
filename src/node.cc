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

void Node::remove_link(const Link link) {
    for (size_t i = 0; i < neighbors_.size(); ++i) {
        if (neighbors_.at(i) == link) {
            neighbors_.at(i) = std::move(neighbors_.back());
            neighbors_.pop_back();
            i--; // need to check same index again
        }
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