#include "node_team.h"

#include <sstream>

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
                                        node_ptr->prototype_->radius;
        if (sq_com_dist < (required_com_dist * required_com_dist)) {
            return true;
        }
    }

    return false;
}

void NodeTeam::check_work_area(NodeTeam const& other) const {
    NICE_PANIC(
        &work_area_ != &other.work_area_,
        string_format(
            "Trying to move NodeTeam of "
            "different work area: %s(%p) and %s(%p)\n",
            work_area_.name().c_str(),
            &work_area_,
            other.work_area_.name().c_str(),
            &other.work_area_));
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

    for (auto& proto_chain : new_node->prototype_->chains()) {
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

void NodeTeam::remove_free_chains(Node* node) {
    // Remove any FreeChain originating from node
    free_chains_.lift_erase_all(
        FreeChain(node, TerminusType::NONE, 0),
        compare_free_chain_nodes);
}

/* public */
/* ctors */
NodeTeam::NodeTeam(WorkArea const& work_area) :
    work_area_(work_area) {
    nodes_.reserve(work_area.target_size());
    free_chains_.reserve(FREE_CHAIN_VM_RESERVE_SIZE);
}

NodeTeam::NodeTeam(NodeTeam const& other) :
    NodeTeam(other.work_area_) {
    *this = other; // Calls operator=(T const&)
}

NodeTeam::NodeTeam(NodeTeam&& other) :
    NodeTeam(other.work_area_) {
    *this = std::move(other); // Calls operator=(T&&)
}

/* dtors */
NodeTeam::~NodeTeam() {
    disperse();
}

/* accessors */
NodeTeamSP NodeTeam::clone() const {
    return NodeTeamSP(clone_impl());
}

/* modifiers */
NodeTeam& NodeTeam::operator=(NodeTeam const& other) {
    if (this != &other) {
        disperse();

        // Clone nodes (heap) and create address mapping
        std::vector<Node *> new_nodes;
        NodeAddrMap addr_map; // old addr -> new addr

        for (auto node_ptr : other.nodes()) {
            new_nodes.push_back(node_ptr->clone());
            addr_map[node_ptr] = new_nodes.back();
        }

        free_chains_ = other.free_chains();

        // Fix pointer addresses and assign to my own nodes
        for (auto node_ptr : new_nodes) {
            node_ptr->update_link_ptrs(addr_map);
            nodes_.push_back(node_ptr);
        }

        for (auto& fc : free_chains_) {
            fc.node = addr_map.at(fc.node);
        }

        checksum_ = other.checksum_;
        score_ = other.score_;
    }

    return *this;
}

NodeTeam& NodeTeam::operator=(NodeTeam&& other) {
    if (this != &other) {
        NICE_PANIC(&work_area_ != &other.work_area_);

        disperse();

        std::swap(nodes_, other.nodes_);
        std::swap(free_chains_, other.free_chains_);

        std::swap(checksum_, other.checksum_);
        std::swap(score_, other.score_);
    }

    return *this;
}

}  /* elfin */