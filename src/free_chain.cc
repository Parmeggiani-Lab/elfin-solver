#include "free_chain.h"

#include "node.h"

namespace elfin {

/* ctors */
FreeChain::FreeChain(
    Node* _node,
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
        node->prototype()->chains().at(chain_id);

    return proto_chain.pick_random_link(term);
}

FreeChain::BridgeList FreeChain::find_bridges(
    FreeChain const& dst) const {
    BridgeList res;
    ProtoModule const* dst_mod = dst.node->prototype();
    const size_t dst_chain_id = dst.chain_id;

    ProtoTerminus const& ptterm_src =
        node->prototype()->chains().at(chain_id).get_term(term);

    // For each middle ProtoModule that ptterm_src connects to...
    for (ProtoLink const* ptlink1 : ptterm_src.links()) {
        ProtoModule const* const middle_mod = ptlink1->module();

        // Skip non basic modules
        if (middle_mod->counts().all_interfaces() > 2) {
            continue;
        }

        size_t const chain_in = ptlink1->chain_id();

        // Look for ptlink2 to dst_mod
        for (ProtoChain const& middle_chain : middle_mod->chains()) {
            // Skip incoming chain.
            if (middle_chain.id == chain_id) continue;

            // dst.term is incoming term, the opposite of which is term.
            ProtoTerminus const& ptterm_out =
                middle_chain.get_term(term);
            ProtoLinkPtrSetCItr itr =
                ptterm_out.find_link_to(dst_mod, dst_chain_id);

            if (itr != ptterm_out.link_set().end()) {
                // We have found a ptlink2
                res.emplace_back(ptlink1, *itr);
            }
        }
    }

    return res;
}

ProtoLink const* FreeChain::find_link_to(
    FreeChain const& dst) const {
    if (dst.term != opposite_term(term)) {
        return nullptr;
    }

    return node->prototype()->find_link_to(
               chain_id,
               term,
               dst.node->prototype(),
               dst.chain_id);
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