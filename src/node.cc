#include "node.h"

#include <sstream>
#include <algorithm>

namespace elfin {

/* public */
/* ctors */
Node::Node(
    ProtoModule const* prototype,
    Transform const& tx) :
    prototype_(prototype),
    tx_(tx) {
    // Reserve memory for maximum occupancy
    links_.reserve(prototype_->counts().all_interfaces());
}

/* accessors */
Link const* Node::find_link_to(Node const* dst_node) const {
    for (auto & link : links_) {
        if (link.dst().node == dst_node) return &link;
    }
    return nullptr;
}

/* modifiers */
void Node::update_link_ptrs(NodeAddrMap const& nam) {
    for (Link & link : links_) {
        link.update_node_ptrs(nam);
    }
}

void Node::remove_link(Link const& link) {
    for (size_t i = 0; i < links_.size(); ++i) {
        if (links_.at(i) == link) {
            links_.at(i) = std::move(links_.back());
            links_.pop_back();
            break; // No two identical links should co-exist!
        }
    }
}

/* printers */
std::string Node::to_string() const {
    return string_format("Node[%s]\nTx: %s",
                         prototype_->name.c_str(),
                         tx_.to_string().c_str());
}

std::string Node::to_csv_string() const {
    UNIMPLEMENTED(); // This function probably need an update
    return tx_.to_csv_string();
}

}  /* elfin */