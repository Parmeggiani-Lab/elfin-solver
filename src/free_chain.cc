#include "free_chain.h"

#include "node.h"

namespace elfin {

/* ctors */
FreeChain::FreeChain(
    Node * _node,
    TerminusType const _term,
    size_t const _chain_id) :
    node(_node),
    term(_term),
    chain_id(_chain_id) {

}

/* accessors */
bool FreeChain::operator==(FreeChain const& other) const {
    return node == other.node and
           term == other.term and
           chain_id == other.chain_id;
}

ProtoLink const& FreeChain::random_proto_link() const {
    ProtoChain const& proto_chain =
        node->prototype()->proto_chains().at(chain_id);

    return proto_chain.pick_random_proto_link(term);
}

/* printers */
std::string FreeChain::to_string() const {
    return string_format("FreeChain[node: %s (%p), term: %s, chain: %lu]",
                         node->prototype()->name.c_str(),
                         node,
                         TerminusTypeToCStr(term),
                         chain_id);
}

}  /* elfin */