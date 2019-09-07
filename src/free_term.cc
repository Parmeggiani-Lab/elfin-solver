#include "free_term.h"

#include "node.h"

namespace elfin {

/* ctors */
FreeTerm::FreeTerm(NodeKey const _node,
                   size_t const _chain_id,
                   TermType const _term) :
    node(_node),
    chain_id(_chain_id),
    term(_term) {

}

/* accessors */
bool FreeTerm::operator==(FreeTerm const& other) const {
    return node == other.node and
           term == other.term and
           chain_id == other.chain_id;
}

ProtoLink const& FreeTerm::random_proto_link(uint32_t& seed) const {
    return get_ptterm().pick_random_link(term, seed);
}

FreeTerm::BridgeList FreeTerm::find_bridges(FreeTerm const& dst) const
{
    BridgeList res;
    ProtoModule const* dst_mod = dst.node->prototype_;
    size_t const dst_chain_id = dst.chain_id;

    ProtoTerm const& ptterm_src =
        node->prototype_->chains().at(chain_id).get_term(term);

    // For each middle ProtoModule that ptterm_src connects to...
    for (auto& ptlink1 : ptterm_src.links()) {
        auto const mid_mod_ptr = ptlink1->module;

        // Skip non basic modules
        if (mid_mod_ptr->counts().all_interfaces() > 2) {
            continue;
        }

        size_t const chain_in = ptlink1->chain_id;

        // Look for ptlink2 to dst_mod
        for (ProtoChain const& middle_chain : mid_mod_ptr->chains()) {
            // Skip incoming chain.
            if (middle_chain.id == chain_id) continue;

            // dst.term is incoming term, the opposite of which is term.
            ProtoTerm const& ptterm_out =
                middle_chain.get_term(term);
            auto pt_link = ptterm_out.find_link_to(dst_mod, dst_chain_id, opposite_term(term));

            if (pt_link) {
                res.emplace_back(ptlink1.get(), pt_link);
            }
        }
    }

    return res;
}

ProtoLink const* FreeTerm::find_link_to(FreeTerm const& dst) const
{
    if (dst.term != opposite_term(term)) {
        return nullptr;
    }

    return node->prototype_->find_link_to(chain_id,
                                          term,
                                          dst.node->prototype_,
                                          dst.chain_id);
}

ProtoTerm const& FreeTerm::get_ptterm() const {
    if (not node) {
        throw BadArgument(std::string("Tried to call ") + __PRETTY_FUNCTION__ + " when node is nullptr.\n");
    }

    return node->prototype_->get_term(*this);
}

bool FreeTerm::nodeless_compare(FreeTerm const& other) const {
    return term == other.term and
           chain_id == other.chain_id;
}

/* printers */
void FreeTerm::print_to(std::ostream& os) const {
    os << "FreeTerm[node: ";
    if (node) {
        os << node->prototype_->name;
    }
    else {
        os << "nil";
    }

    os << " (Ptr: " << (void*) node << "), ";
    os << "term: " << TermTypeToCStr(term) << ", ";
    os << "chain: " << chain_id << "]";
}

}  /* elfin */