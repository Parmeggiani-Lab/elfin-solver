#include "node.h"

#include <sstream>
#include <algorithm>

namespace elfin {

/* public */
Node::Node(const Module * prototype, const Transform & tx) :
    prototype_(prototype), tx_(tx) {
    for (auto & chain : prototype_->chains) {
        const size_t chain_id = prototype_->chain_id_map.at(chain.name);

        if (chain.n_links.size() > 0) {
#ifdef NDEBUG
            if (!term_tracker_.is_free(TerminusType::N, chain_id)) {
                die(("Tried to add chain_id[%lu] that already "
                     "exists in N terminus chain ID list of\n\t%s\n"),
                    to_string().c_str());
            }
#endif  /* ifdef NDEBUG */

            term_tracker_.get_free(TerminusType::N).push_back(chain_id);
        }
        if (chain.c_links.size() > 0) {
#ifdef NDEBUG
            if (!term_tracker_.is_free(TerminusType::C, chain_id)) {
                die(("Tried to add chain_id[%lu] that already "
                     "exists in C terminus chain ID list of\n\t%s\n"),
                    to_string().c_str());
            }
#endif  /* ifdef NDEBUG */

            term_tracker_.get_free(TerminusType::C).push_back(chain_id);
        }
    }
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