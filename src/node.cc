#include "node.h"

#include <sstream>
#include <algorithm>

#include "debug_utils.h"
#include "stack_trace.h"

#include "path_generator.h"

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
Link const* Node::find_link_to(NodeKey dst_node) const {
    for (auto& link : links_) {
        if (link.dst().node == dst_node) {
            return &link;
        }
    }
    return nullptr;
}

PathGenerator Node::gen_path() const {
    return PathGenerator(this);
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
        JUtil.error("Trying to remove link that does not exist. Links:\n");
        for (size_t i = 0; i < links_.size(); ++i) {
            JUtil.error("Link #%zu: %s\n",
                        i, links_[i].to_string().c_str());
        }
        JUtil.error("%s\n", to_string().c_str());
        JUtil.panic("%s\n", src.to_string().c_str());
    }
}

/* printers */
void Node::print_to(std::ostream& os) const {
    os << "Node(" << prototype_->name << ":";
    os << (void*) this << ")\n";
    os << tx_.to_string();
}

}  /* elfin */