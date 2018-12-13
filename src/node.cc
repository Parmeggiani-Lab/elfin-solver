#include "node.h"

#include <sstream>
#include <algorithm>

#include "debug_utils.h"

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
    for (auto& link : links_) {
        if (link.dst().node == dst_node) return &link;
    }
    return nullptr;
}

/* modifiers */
void Node::update_link_ptrs(NodeAddrMap const& nam) {
    for (Link& link : links_) {
        link.update_node_ptrs(nam);
    }
}

void Node::remove_link(FreeChain const& src) {
    bool found_link = false;
    for (size_t i = 0; i < links_.size(); ++i) {
        if (links_.at(i).src() == src) {
            links_.at(i) = std::move(links_.back());
            links_.pop_back();
            found_link = true;
            break; // No two identical links should co-exist!
        }
    }

    // NICE_PANIC(not found_link,
    //     string_format("Link not found: %s\n", link.to_string().c_str()));
    NICE_PANIC(not found_link);
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