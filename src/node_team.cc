#include "node_team.h"

#include <sstream>

#define FREE_CHAIN_VM_RESERVE_SIZE 16

namespace elfin {

/* free methods */
bool compare_free_chain_nodes(
    const FreeChain & a,
    const FreeChain & b) {
    return a.node == b.node;
}

/* protected */
Node * NodeTeam::add_member(
    const ProtoModule * prot,
    const Transform & tx) {
    Node * new_node = new Node(prot, tx);
    nodes_.push_back(new_node);

    for (auto & proto_chain : new_node->prototype()->proto_chains()) {
        const size_t chain_id =
            new_node->prototype()->chain_id_map().at(proto_chain.name);

        if (proto_chain.n_term().proto_links().size() > 0) {
            free_chains_.emplace_back(new_node, TerminusType::N, chain_id);
        }

        if (proto_chain.c_term().proto_links().size() > 0) {
            free_chains_.emplace_back(new_node, TerminusType::C, chain_id);
        }
    }

    return new_node;
}

void NodeTeam::remove_member(const Node * const_node) {
    Node * node =  const_cast<Node *>(const_node);
    nodes_.erase(node);

    // Remove any FreeChain originating from sever_point
    free_chains_.lift_erase_all(
        FreeChain(node, TerminusType::NONE, 0),
        compare_free_chain_nodes);

    delete node;
}

/* public */
NodeTeam::NodeTeam() {
    free_chains_.reserve(FREE_CHAIN_VM_RESERVE_SIZE);
}

NodeTeam::NodeTeam(
    NodeTeam && other) {
    *this = other; // Call operator=(T&&)
}

const ProtoLink & NodeTeam::random_proto_link(
    const FreeChain & free_chain) const {
    const ProtoChain & proto_chain =
        free_chain.node->prototype()->
        proto_chains().at(free_chain.chain_id);

    return proto_chain.pick_random_proto_link(free_chain.term);
}

NodeTeam & NodeTeam::operator=(NodeTeam && other) {
    if (this != &other) {
        disperse();
        // Take over the already allocated nodes
        nodes_ = other.nodes_;
        free_chains_ = other.free_chains_;
        other.nodes_.clear();
        other.free_chains_.clear();
    }
    return *this;
}

NodeTeam::~NodeTeam() {
    disperse();
}

void NodeTeam::disperse() {
    for (auto node_ptr : nodes_) {
        delete node_ptr;
    }

    nodes_.clear();
    free_chains_.clear();
}

void NodeTeam::remake(const Roulette<ProtoModule *> & mod_list) {
    disperse();

    // Pick random initial member
    const ProtoModule * prot = mod_list.draw();
    add_member(prot);
}

const Node * NodeTeam::invite_new_member(
    const FreeChain free_chain_a,
    const ProtoLink & proto_link) {

    const TerminusType term_a = free_chain_a.term;
    const TerminusType term_b = OPPOSITE_TERM[term_a];

    Node * node_a = free_chain_a.node;
    Node * node_b = add_member(
                        proto_link.target_mod,
                        node_a->tx() * proto_link.tx);

    const FreeChain free_chain_b = FreeChain(
                                       node_b,
                                       term_b,
                                       proto_link.target_chain_id);

    node_a->add_link(free_chain_a, free_chain_b);
    node_b->add_link(free_chain_b, free_chain_a);

    free_chains_.lift_erase(free_chain_a);
    free_chains_.lift_erase(free_chain_b);

    return node_b;
}

}  /* elfin */