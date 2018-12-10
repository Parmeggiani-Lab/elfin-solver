#include "free_chain.h"

#include "node.h"

namespace elfin {

FreeChain::FreeChain(
    Node * _node,
    const TerminusType _term,
    const size_t _chain_id) :
    node(_node),
    term(_term),
    chain_id(_chain_id) {

}

bool FreeChain::operator==(const FreeChain & other) const {
    return node == other.node and
           term == other.term and
           chain_id == other.chain_id;
}

std::string FreeChain::to_string() const {
    return string_format("FreeChain[node: %s (%p), term: %s, chain: %lu]",
                         node->prototype()->name.c_str(),
                         node,
                         TerminusTypeToCStr(term),
                         chain_id);
}

}  /* elfin */