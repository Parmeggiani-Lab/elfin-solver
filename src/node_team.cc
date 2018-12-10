#include "node_team.h"

#include <sstream>

#include "candidate.h"

#define FREE_CHAIN_VM_RESERVE_SIZE 16

namespace elfin {

/* free methods */
bool compare_free_chain_nodes(
    const FreeChain & a,
    const FreeChain & b) {
    return a.node == b.node;
}

/* protected */

/* accessors */
const ProtoLink & NodeTeam::random_proto_link(
    const FreeChain & free_chain) const {
    const ProtoChain & proto_chain =
        free_chain.node->prototype()->
        proto_chains().at(free_chain.chain_id);

    return proto_chain.pick_random_proto_link(free_chain.term);
}

bool NodeTeam::collides(
    const Vector3f & new_com,
    const float mod_radius) const {

    for (const auto node_ptr : nodes_) {
        const float sq_com_dist = node_ptr->tx_.collapsed().sq_dist_to(new_com);
        const float required_com_dist = mod_radius +
                                        node_ptr->prototype()->radius;
        if (sq_com_dist < (required_com_dist * required_com_dist)) {
            return true;
        }
    }

    return false;
}

/* modifiers */
void NodeTeam::disperse() {
    for (auto node_ptr : nodes_) {
        delete node_ptr;
    }

    nodes_.clear();
    free_chains_.clear();
}

Node * NodeTeam::add_member(
    const ProtoModule * prot,
    const Transform & tx) {
    Node * new_node = new Node(prot, tx);
    nodes_.push_back(new_node);

    for (auto & proto_chain : new_node->prototype()->proto_chains()) {
        if (not proto_chain.n_term().proto_links().empty()) {
            free_chains_.emplace_back(new_node, TerminusType::N, proto_chain.id);
        }

        if (not proto_chain.c_term().proto_links().empty()) {
            free_chains_.emplace_back(new_node, TerminusType::C, proto_chain.id);
        }
    }

    return new_node;
}

const Node * NodeTeam::invite_new_member(
    const FreeChain free_chain_a,
    const ProtoLink & proto_link) {

    const TerminusType term_a = free_chain_a.term;
    const TerminusType term_b = OPPOSITE_TERM[static_cast<int>(term_a)];

    Node * node_a = free_chain_a.node;
    Node * node_b = add_member(
                        proto_link.target_mod,
                        node_a->tx_ * proto_link.tx);
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

void NodeTeam::remove_member(Node * node) {
    nodes_.erase(node);
    delete node;
}

void NodeTeam::remove_member_chains(Node * node) {
    // Remove any FreeChain originating from node
    free_chains_.lift_erase_all(
        FreeChain(node, TerminusType::NONE, 0),
        compare_free_chain_nodes);
}

/* public */
/* ctors */
NodeTeam::NodeTeam() {
    nodes_.reserve(Candidate::MAX_LEN);
    free_chains_.reserve(FREE_CHAIN_VM_RESERVE_SIZE);
}

NodeTeam::NodeTeam(NodeTeam && other) {
    *this = other; // Call operator=(T&&)
}

/* dtors */
NodeTeam::~NodeTeam() {
    disperse();
}

/* accessors */

/* modifiers */
NodeTeam & NodeTeam::operator=(NodeTeam && other) {
    if (this != &other) {
        disperse();

        // Take over the already allocated resourcse
        nodes_ = other.nodes_;
        free_chains_ = other.free_chains_;

        other.nodes_.clear();
        other.free_chains_.clear();
    }

    return *this;
}

}  /* elfin */