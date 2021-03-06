#include "node.h"

#include <sstream>
#include <algorithm>

#include "debug_utils.h"
#include "stack_trace.h"

#include "path_generator.h"

namespace elfin {

/* public */
/* accessors */
Link const* Node::find_link_to(NodeKey dst_node) const {
    for (auto& link : links_) {
        if (link.dst().node == dst_node) {
            return &link;
        }
    }
    return nullptr;
}

/* modifiers */
void Node::update_link_ptrs(NodeKeyMap const& nam) {
    for (Link& link : links_) {
        link.update_node_ptrs(nam);
    }
}

void Node::remove_link(FreeTerm const& src) {
    auto link_itr = std::find_if(
                        begin(links_),
                        end(links_),
    [&](auto const & link) { return link.src() == src; });

    // Verbose diagnosis
    if (link_itr == end(links_)) {
        print_stacktrace();

        std::ostringstream oss;
        oss << "Attempted to remove a link that does not exist. Links:\n";

        size_t index = 0;
        for (auto& link : links_) {
            oss << "Link #" << index++ << ": ";
            oss << link << "\n";
        }

        PANIC(BadArgument(std::string(__PRETTY_FUNCTION__) + " failed:\n" +
                          "src=" + src.to_string() + "\n" +
                          "Node: " + this->to_string() + "\n" +
                          oss.str()));
    }
    else {
        links_.erase(link_itr);
    }
}

/* printers */
void Node::print_to(std::ostream& os) const {
    os << "Node (" << prototype_->name << ":";
    os << (void*) this << ") [\n";
    os << tx_ << "\n";
    os << "]\n";
}

}  /* elfin */