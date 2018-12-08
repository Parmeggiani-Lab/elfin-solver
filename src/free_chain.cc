#include "free_chain.h"

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
    return node == other.node and chain_id == other.chain_id;
}

std::string FreeChain::to_string() const {
    return string_format("FreeChain[node: %p, term: %s, chain: %lu\n",
                         node,
                         TerminusTypeNames[term],
                         chain_id);
}

std::string FreeChainVM::to_string() const {
    std::ostringstream ss;
    for (auto & sk : items_) {
        ss << sk.to_string();
    }
    return ss.str();
}


}  /* elfin */