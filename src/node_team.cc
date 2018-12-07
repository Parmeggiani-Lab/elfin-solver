#include "node_team.h"

#include <sstream>

#define SEEKER_VM_RESERVE_SIZE 16

namespace elfin {

/* protected */
std::string NodeTeam::SeekerVectorMap::to_string() const {
    std::ostringstream ss;
    for (auto & sk : items_) ss << sk.to_string();
    return ss.str();
}

TerminusType NodeTeam::SeekerVMPair::get_term(
    const ChainSeeker & seeker) const {
    // Could store this as a 3rd field in ChainSeeker, but that requires one
    // extra byte per seeker... The following is O(1).
    TerminusType term = TerminusType::NONE;
    if (n.has(seeker))  {
        term = TerminusType::N;
    }
    else if (c.has(seeker))  {
        term = TerminusType::C;
    }
    else {
        err("Seeker does not belong to any SeekerVectorMap!\n");
        err("Seeker: %s\n", seeker.to_string().c_str());
        err(to_string().c_str());
        NICE_PANIC(true, string_format("%s failed\n", __PRETTY_FUNCTION__));
    }

    return term;
}

NodeTeam::SeekerVectorMap & NodeTeam::SeekerVMPair::get_vm(
    const TerminusType term) {
    if (term == TerminusType::N) {
        return n;
    }

    if (term == TerminusType::C) {
        return c;
    }

    death_by_bad_terminus(__PRETTY_FUNCTION__, term);
    exit(1); // Suppress warning
}

std::string NodeTeam::SeekerVMPair::to_string() const {
    std::ostringstream ss;
    ss << "SeekerVMPair[\n";
    ss << "\tN-term:\n" << n.to_string();
    ss << "\tC-term:\n" << c.to_string();
    return ss.str();
}

// static
NodeTeam::NodeVM NodeTeam::copy_nodes_from(const NodeTeam::NodeVM & other) {
    NodeTeam::NodeVM new_nodes;

    // Need to make clones of nodes from other NodeTeam
    for (auto node_ptr : other.items()) {
        new_nodes.push_back(node_ptr->clone());
    }

    return new_nodes;
}

Node * NodeTeam::add_member(
    const ProtoModule * prot,
    const Transform & tx) {
    Node * new_node = new Node(prot, tx);
    nodes_.push_back(new_node);

    for (auto & proto_chain : new_node->prototype()->proto_chains()) {
        const size_t chain_id =
            new_node->prototype()->chain_id_map().at(proto_chain.name);

        if (proto_chain.n_term().proto_links().size() > 0) {
            free_seekers_.n.emplace_back(new_node, chain_id);
        }

        if (proto_chain.c_term().proto_links().size() > 0) {
            free_seekers_.c.emplace_back(new_node, chain_id);
        }
    }

    return new_node;
}

/* public */
NodeTeam::NodeTeam() {
    free_seekers_.n.reserve(SEEKER_VM_RESERVE_SIZE);
    free_seekers_.c.reserve(SEEKER_VM_RESERVE_SIZE);
}

NodeTeam::NodeTeam(const NodeTeam & other) :
    nodes_(copy_nodes_from(other.nodes_)),
    free_seekers_(other.free_seekers_) {
}

NodeTeam::NodeTeam(NodeTeam && other) :
    free_seekers_(other.free_seekers_) {
    // Take over the already allocated nodes
    nodes_ = other.nodes_;
    other.nodes_.clear();
}

const ChainSeeker & NodeTeam::random_free_seeker(
    TerminusType term) const {
    if (term == TerminusType::ANY) {
        if (free_seekers_.n.empty()) {
            term = TerminusType::C;
        }
        else if (free_seekers_.c.empty()) {
            term = TerminusType::N;
        }
        else {
            term = random_term();
        }
    }

    if (term == TerminusType::N) {
        const ChainSeeker & rand_n_seeker =
            free_seekers_.n.rand_item();
        return rand_n_seeker;
    }

    if (term == TerminusType::C) {
        const ChainSeeker & rand_c_seeker =
            free_seekers_.c.rand_item();
        return rand_c_seeker;
    }

    death_by_bad_terminus(__PRETTY_FUNCTION__, term);
    exit(1); // Suppress warning
}

const ProtoLink & NodeTeam::random_proto_link(
    const ChainSeeker & seeker) const {
    const ProtoChain & proto_chain =
        seeker.node->prototype()->proto_chains().at(seeker.chain_id);

    // Assuming seeker is from free_seekers_
    const TerminusType term = free_seekers_.get_term(seeker);
    return proto_chain.pick_random_proto_link(term);
}

NodeTeam & NodeTeam::operator=(const NodeTeam & other) {
    if (this != &other) {
        disperse();
        nodes_ = copy_nodes_from(other.nodes_);
        free_seekers_ = other.free_seekers_;
    }
    return *this;
}

NodeTeam & NodeTeam::operator=(NodeTeam && other) {
    if (this == &other) {
        disperse();
        // Take over the already allocated nodes
        nodes_ = other.nodes_;
        other.nodes_.clear();
        free_seekers_ = other.free_seekers_;
    };
    return *this;
}

NodeTeam::~NodeTeam() {
    disperse();
}

void NodeTeam::disperse() {
    for (auto node_ptr : nodes_.items()) {
        delete node_ptr;
    }

    nodes_.clear();
    free_seekers_.n.clear();
    free_seekers_.c.clear();
}

void NodeTeam::remake(const Roulette<ProtoModule *> & mod_list) {
    disperse();

    // Pick random initial member
    const ProtoModule * prot = mod_list.draw();
    add_member(prot);
}

const Node * NodeTeam::invite_new_member(
    const ChainSeeker seeker_a,
    const ProtoLink & proto_link) {

    Node * node_a = seeker_a.node;
    Node * node_b = add_member(proto_link.mod, node_a->tx() * proto_link.tx);

    DEBUG(node_a == node_b);

    const ChainSeeker seeker_b = ChainSeeker(node_b, proto_link.target_chain_id);
    const TerminusType term_a = free_seekers_.get_term(seeker_a);
    const TerminusType term_b = OPPOSITE_TERM[term_a];

    wrn("node_a[%p] adding seeker %s",
        node_a, seeker_b.to_string().c_str());
    node_a->store_seeker(seeker_b, term_a);
    wrn("now %lu neighbors\n\n", node_a->n_neighbors().size() +
        node_a->c_neighbors().size());

    wrn("node_b[%p] adding seeker %s",
        node_b, seeker_a.to_string().c_str());
    node_b->store_seeker(seeker_a, term_b);
    wrn("now %lu neighbors\n\n", node_b->n_neighbors().size() +
        node_b->c_neighbors().size());

    free_seekers_.get_vm(term_a).erase(seeker_a);
    free_seekers_.get_vm(term_b).erase(seeker_b);

    return node_b;
}

}  /* elfin */