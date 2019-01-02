#include "free_term.h"

#include "node.h"

namespace elfin {

/* ctors */
FreeTerm::FreeTerm(NodeKey const _node,
                     TermType const _term,
                     size_t const _chain_id) :
    node(_node),
    term(_term),
    chain_id(_chain_id) {

}

/* accessors */
bool FreeTerm::operator==(FreeTerm const& other) const {
    return node == other.node and
           term == other.term and
           chain_id == other.chain_id;
}

ProtoLink const& FreeTerm::random_proto_link() const {
    ProtoChain const& proto_chain =
        node->prototype_->chains().at(chain_id);

    return proto_chain.pick_random_link(term);
}

FreeTerm::BridgeList FreeTerm::find_bridges(
    FreeTerm const& dst) const
{
    BridgeList res;
    ProtoModule const* dst_mod = dst.node->prototype_;
    size_t const dst_chain_id = dst.chain_id;

    ProtoTerm const& ptterm_src =
        node->prototype_->chains().at(chain_id).get_term(term);

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
            ProtoTerm const& ptterm_out =
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

ProtoLink const* FreeTerm::find_link_to(
    FreeTerm const& dst) const
{
    if (dst.term != opposite_term(term)) {
        return nullptr;
    }

    return node->prototype_->find_link_to(
               chain_id,
               term,
               dst.node->prototype_,
               dst.chain_id);
}

/* printers */
void FreeTerm::print_to(std::ostream& os) const {
    os << "FreeTerm[node: " << node->prototype_->name;
    os << " (Ptr: " << (void*) node << "), ";
    os << "term: " << TermTypeToCStr(term) << ", ";
    os << "chain: " << chain_id << "]";
}

}  /* elfin */