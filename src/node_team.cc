#include "node_team.h"

#include <sstream>

#include "candidate.h"

#define FREE_CHAIN_VM_RESERVE_SIZE 16

namespace elfin {

/* free methods */
bool compare_free_chain_nodes(
    FreeChain const& a,
    FreeChain const& b) {
    return a.node == b.node;
}

/* protected */
/* accessors */
bool NodeTeam::collides(
    Vector3f const& new_com,
    float const mod_radius) const {

    for (auto const node_ptr : nodes_) {
        float const sq_com_dist = node_ptr->tx_.collapsed().sq_dist_to(new_com);
        float const required_com_dist = mod_radius +
                                        node_ptr->prototype()->radius;
        if (sq_com_dist < (required_com_dist* required_com_dist)) {
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

Node* NodeTeam::add_member(
    ProtoModule const* prot,
    Transform const& tx) {
    Node* new_node = new Node(prot, tx);
    nodes_.push_back(new_node);

    for (auto& proto_chain : new_node->prototype()->chains()) {
        if (not proto_chain.n_term().links().empty()) {
            free_chains_.emplace_back(new_node, TerminusType::N, proto_chain.id);
        }

        if (not proto_chain.c_term().links().empty()) {
            free_chains_.emplace_back(new_node, TerminusType::C, proto_chain.id);
        }
    }

    return new_node;
}

void NodeTeam::remove_member(Node* node) {
    nodes_.erase(node);
    delete node;
}

void NodeTeam::remove_member_chains(Node* node) {
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
    *this = std::move(other); // Call operator=(T&&)
}

/* dtors */
NodeTeam::~NodeTeam() {
    disperse();
}

/* accessors */

/* modifiers */
NodeTeam& NodeTeam::operator=(NodeTeam && other) {
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