#include "basic_node_generator.h"

namespace elfin {

/* public */
/* accessors */
std::vector<Link const*> BasicNodeGenerator::collect_arrows(
    NodeSP const& start_node) {
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
NodeSP BasicNodeGenerator::next() {
    NodeSP prev_node = curr_node_;
    curr_node_ = next_node_;
    next_node_ = nullptr;
    curr_link_ = nullptr;

    // Look for next node
    if (curr_node_) {
        size_t const num_neighbors = curr_node_->links().size();
        NICE_PANIC(num_neighbors > 2);
        for (auto& link : curr_node_->links()) {
            NodeSP sp = link.dst().node_sp();
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