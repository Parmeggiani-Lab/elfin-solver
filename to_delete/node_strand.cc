#include "node_strand.h"

#include "random_utils.h"

namespace elfin {

NodeStrand::NodeStrand(const ProtoModule * start_mod) {
    nodes_.emplace_back(start_mod);
}


V3fList to_points() const {
    V3fList res;

    for (const Node & n : nodes_) {
        res.emplace_back(n->tx().collapsed());
    }

    return res;
}

const Node & random_tip() const {
    DEBUG(nodes_.size() == 0);
    return get_dice(2) == 0 ? nodes_.front() : nodes_.back();
}

StrList get_node_names() const {
    StrList res;

    for (const Node & n : nodes_) {
        res.emplace_back(n->prototype()->name);
    }

    return res;
}

}  /* elfin */