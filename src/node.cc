#include "node.h"

#include <sstream>
#include <algorithm>

namespace elfin {

/* public */
Node::Node(const Module * prototype, const Transform & tx) :
    prototype_(prototype),
    tx_(tx),
    term_tracker_(prototype) {
}

/*
 * Occupy specified termini on specified chains of a pair of nodes.
 * (static)
 */
void Node::connect(
    Node * node_a,
    const size_t a_chain_id,
    const TerminusType a_term,
    Node * node_b,
    const size_t b_chain_id) {
    node_a->term_tracker_.occupy_terminus(a_term, a_chain_id);
    node_b->term_tracker_.occupy_terminus(OPPOSITE_TERM[a_term], b_chain_id);
}

std::string Node::to_string() const {
    std::stringstream ss;
    ss << "Node[" << prototype_->name << "]" << std::endl;
    ss << "Tx: " << tx_.to_string();
    return ss.str();
}

std::string Node::to_csv_string() const {
    die("Revise this function?\n");
    return tx_.to_csv_string();
}

}  /* elfin */