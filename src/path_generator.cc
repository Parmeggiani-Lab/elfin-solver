#include "path_generator.h"

#include "link.h"
#include "node.h"

namespace elfin {

/* public */
/* ctors */
PathGenerator::PathGenerator(Link const* const arrow) :
    curr_node_(arrow->src().node),
    curr_link_(arrow),
    next_node_(arrow->dst().node) {}

/* accessors */
std::vector<Link const*> PathGenerator::collect_arrows() {
    std::vector<Link const*> arrows;

    while (not is_done()) {
        next();
        if (curr_link()) {
            arrows.push_back(curr_link());
        }
    }

    return arrows;
}

/* modifiers */
NodeKey PathGenerator::next()
{
    NodeKey prev_node = curr_node_;
    curr_node_ = next_node_;
    next_node_ = nullptr;
    curr_link_ = nullptr;

    // Look for next node
    if (curr_node_) {
        size_t const num_neighbors = curr_node_->links().size();
        DEBUG_NOMSG(num_neighbors > 2);
        DEBUG_NOMSG(num_neighbors == 0);

        for (auto& link : curr_node_->links()) {
            NodeKey sp = link.dst().node;
            if (sp != prev_node) {
                // curr_link links curr_node to next_node
                curr_link_ = &link;
                next_node_ = sp;
                break;
            }
        }
    }

    return curr_node_;
}

}  /* elfin */