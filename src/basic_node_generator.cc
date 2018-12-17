#include "basic_node_generator.h"

namespace elfin {

/* public */
/* accessors */
std::vector<Link const*> BasicNodeGenerator::collect_arrows(
    Node* start_node) {
    BasicNodeGenerator gen(start_node);
    std::vector<Link const*> arrows;

    while (not gen.is_done()) {
        gen.next();
        if (gen.curr_link()) {
            arrows.push_back(gen.curr_link());
        }
    }

    return arrows;
}

/* modifiers */
Node* BasicNodeGenerator::next() {
    Node* prev_node = curr_node_;
    curr_node_ = next_node_;

    next_node_ = nullptr;
    curr_link_ = nullptr;

    // Look for next node
    if (curr_node_) {
        size_t const neighbor_size = curr_node_->links().size();
        NICE_PANIC(neighbor_size > 2);
        for (auto& link : curr_node_->links()) {
            if (link.dst().node != prev_node) {
                // curr_link links curr_node to next_node
                curr_link_ = &link;
                next_node_ = link.dst().node;
                break;
            }
        }
    }

    return curr_node_;
}

}  /* elfin */