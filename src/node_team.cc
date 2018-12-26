#include "node_team.h"

#include <sstream>

#define FREE_CHAIN_VM_RESERVE_SIZE 16

namespace elfin {

/* free methods */
bool compare_free_chain_nodes(
    FreeChain const& a,
    FreeChain const& b) {
    return a.node_sp() == b.node_sp();
}

/* protected */
/* accessors */
// bool NodeTeam::collides(
//     Vector3f const& new_com,
//     float const mod_radius) const {

//     for (auto& node : nodes_) {
//         float const sq_com_dist = node->tx_.collapsed().sq_dist_to(new_com);
//         float const required_com_dist =
//             mod_radius + node->prototype_->radius;
//         if (sq_com_dist < (required_com_dist * required_com_dist)) {
//             return true;
//         }
//     }

//     return false;
// }

/* modifiers */
void NodeTeam::reset() {
    work_area_ = nullptr;
    nodes_.clear();
    free_chains_.clear();
    checksum_ = 0x0000;
    score_ = INFINITY;
}

NodeSP NodeTeam::add_member(
    ProtoModule const* prot,
    Transform const& tx) {
    auto new_node = std::make_shared<Node>(prot, tx);

    for (auto& proto_chain : new_node->prototype_->chains()) {
        if (not proto_chain.n_term().links().empty()) {
            free_chains_.emplace_back(new_node, TerminusType::N, proto_chain.id);
        }

        if (not proto_chain.c_term().links().empty()) {
            free_chains_.emplace_back(new_node, TerminusType::C, proto_chain.id);
        }
    }

    nodes_.push_back(new_node);
    return new_node;
}

void NodeTeam::remove_free_chains(NodeSP const& node) {
    // Remove any FreeChain originating from node
    free_chains_.lift_erase_all(
        FreeChain(node, TerminusType::NONE, 0),
        compare_free_chain_nodes);
}

/* public */
/* ctors */
NodeTeam::NodeTeam(WorkArea const* work_area) :
    work_area_(work_area) {
    NICE_PANIC(work_area_ == nullptr);
    nodes_.reserve(work_area_->target_size());
    free_chains_.reserve(FREE_CHAIN_VM_RESERVE_SIZE);
}

NodeTeam::NodeTeam(NodeTeam const& other) {
    *this = other; // Calls operator=(T const&)
}

NodeTeam::NodeTeam(NodeTeam&& other) {
    *this = std::move(other); // Calls operator=(T&&)
}

/* dtors */
NodeTeam::~NodeTeam() {
    reset();
}

/* accessors */
NodeTeamSP NodeTeam::clone() const {
    return NodeTeamSP(clone_impl());
}

/* modifiers */
NodeTeam& NodeTeam::operator=(NodeTeam const& other) {
    if (this != &other) {
        reset();

        // Copy simple fields.
        work_area_ = other.work_area_;
        checksum_ = other.checksum_;
        score_ = other.score_;

        // Clone nodes and create address mapping for remapping pointers.
        {
            NodeAddrMap addr_map; // old addr -> new addr

            for (auto& other_node : other.nodes()) {
                auto node_sp = other_node->clone();
                nodes_.push_back(node_sp);
                addr_map[other_node] = node_sp;
            }

            free_chains_ = other.free_chains();

            // Fix pointer addresses and assign to my own nodes
            for (auto& node : nodes_) {
                node->update_link_ptrs(addr_map);
            }

            for (auto& fc : free_chains_) {
                fc.node = addr_map.at(fc.node_sp());
            }
        }
    }

    return *this;
}

NodeTeam& NodeTeam::operator=(NodeTeam&& other) {
    if (this != &other) {
        reset();

        std::swap(work_area_, other.work_area_);
        std::swap(nodes_, other.nodes_);
        std::swap(free_chains_, other.free_chains_);

        std::swap(checksum_, other.checksum_);
        std::swap(score_, other.score_);
    }

    return *this;
}

}  /* elfin */