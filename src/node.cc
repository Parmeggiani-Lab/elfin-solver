#include "node.h"

#include <sstream>
#include <algorithm>

#include "debug_utils.h"
#include "stack_trace.h"

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
Link const* Node::find_link_to(NodeSP const& dst_node) const {
    for (auto& link : links_) {
        if (link.dst().node_sp() == dst_node) return &link;
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
            links_.at(i) = links_.back();
            links_.pop_back();
            found_link = true;
            break; // No two identical links should co-exist!
        }
    }

    // A bit more verbose diagnosis
    if (not found_link) {
        print_stacktrace();
        err("Trying to remove link that does not exist. Links:\n");
        for (size_t i = 0; i < links_.size(); ++i) {
            err("Link #%zu: %s\n",
                i, links_[i].to_string().c_str());
        }
        err("%s\n", to_string().c_str());
        die("%s\n", src.to_string().c_str());
    }
}

/* printers */
std::string Node::to_string() const {
    return string_format("Node[%s:%p]\n%s",
                         prototype_->name.c_str(),
                         this,
                         tx_.to_string().c_str());
}

std::string Node::to_csv_string() const {
    UNIMPLEMENTED(); // This function needs an update
    return "???";
}

}  /* elfin */