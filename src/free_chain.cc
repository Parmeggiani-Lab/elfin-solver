#include "free_chain.h"

#include "node.h"

namespace elfin {

/* ctors */
FreeChain::FreeChain(
    NodeWP const& _node,
    TerminusType const _term,
    size_t const _chain_id) :
    node(_node),
    term(_term),
    chain_id(_chain_id) {

}

/* accessors */
bool FreeChain::operator==(FreeChain const& other) const {
    return node_sp() == other.node_sp() and
           term == other.term and
           chain_id == other.chain_id;
}

ProtoLink const& FreeChain::random_proto_link() const {
    ProtoChain const& proto_chain =
        node_sp()->prototype_->chains().at(chain_id);

    return proto_chain.pick_random_link(term);
}

FreeChain::BridgeList FreeChain::find_bridges(
    FreeChain const& dst) const {
    BridgeList res;
    ProtoModule const* dst_mod = dst.node_sp()->prototype_;
    size_t const dst_chain_id = dst.chain_id;

    ProtoTerminus const& ptterm_src =
        node_sp()->prototype_->chains().at(chain_id).get_term(term);

    // For each middle ProtoModule that ptterm_src connects to...
    for (auto& ptlink1 : ptterm_src.links()) {
        ProtoModule const* middle_mod = ptlink1->module_;

        // Skip non basic modules
        if (middle_mod->counts().all_interfaces() > 2) {
            continue;
        }

        size_t const chain_in = ptlink1->chain_id_;

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
                res.emplace_back(ptlink1.get(), *itr);
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

    return node_sp()->prototype_->find_link_to(
               chain_id,
               term,
               dst.node_sp()->prototype_,
               dst.chain_id);
}

NodeSP FreeChain::node_sp() const {
    auto sp = node.lock();
    if (sp)
        return sp;
    else
        return nullptr;
}

/* printers */
void FreeChain::print_to(std::ostream& os) const {
    NodeSP sp = node_sp();
    os << "FreeChain[node: " << sp->prototype_->name;
    os << " (Ptr: " << (void*) sp.get() << "), ";
    os << "term: " << TerminusTypeToCStr(term) << ", ";
    os << "chain: " << chain_id << "]";
}

}  /* elfin */