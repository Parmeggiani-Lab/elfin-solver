#include "node_team.h"

#include <sstream>

namespace elfin {

/* protected */

NodeTeam::SeekerVM & NodeTeam::SeekerPair::get_vm(
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

TerminusType NodeTeam::SeekerPair::get_term(
    const ChainSeeker & seeker) const {
    // Could store this as a 3rd field in ChainSeeker, but that requires one extra byte
    // per seeker... The following is O(1).
    TerminusType term = TerminusType::NONE;
    if (n.has(seeker))  {
        term = TerminusType::N;
    }
    else if (c.has(seeker))  {
        term = TerminusType::C;
    }
    else {
        err("Seeker does not belong to any SeekerVM!\n");
        err("Seeker: %s\n", seeker.to_string().c_str());
        err(to_string().c_str());
        NICE_PANIC(true, string_format("%s failed\n", __PRETTY_FUNCTION__));
    }

    return term;
}

std::string NodeTeam::SeekerPair::to_string() const {
    std::ostringstream ss;
    ss << "SeekerPair[\n";
    ss << "\tN-term:\n" << n.to_string();
    ss << "\tC-term:\n" << c.to_string();
    return ss.str();
}

void NodeTeam::copy_nodes_from(const NodeVM & other) {
    // Need to make clones of nodes from other NodeTeam
    for (auto node_ptr : other.items()) {
        nodes_.push_back(node_ptr->clone());
    }
}

const Node * NodeTeam::add_member(
    const Module * prot,
    const Transform & tx) {
    const Node * new_node = new Node(prot, tx);
    nodes_.push_back(new_node);

    for (auto & chain : new_node->prototype()->chains()) {
        const size_t chain_id = new_node->prototype()->chain_id_map().at(chain.name);

        if (chain.n_term().links().size() > 0) {
            free_seekers_.n.emplace_back(new_node, chain_id);
        }

        if (chain.c_term().links().size() > 0) {
            free_seekers_.c.emplace_back(new_node, chain_id);
        }
    }

    return new_node;
}

/* public */
NodeTeam::NodeTeam(const NodeTeam & other) {
    copy_nodes_from(other.nodes_);
}

NodeTeam::NodeTeam(NodeTeam && other) {
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
        const ChainSeeker & rand_n_seeker = free_seekers_.n.rand_item();
        return rand_n_seeker;
    }

    if (term == TerminusType::C) {
        const ChainSeeker & rand_c_seeker = free_seekers_.c.rand_item();
        return rand_c_seeker;
    }

    death_by_bad_terminus(__PRETTY_FUNCTION__, term);
    exit(1); // Suppress warning
}

const Link & NodeTeam::random_link(const ChainSeeker & seeker) const {
    const Chain & chain = seeker.node->prototype()->chains().at(seeker.chain_id);

    // Assuming seeker is from free_seekers_
    const TerminusType term = free_seekers_.get_term(seeker);
    return chain.pick_random_link(term);
}

NodeTeam & NodeTeam::operator=(const NodeTeam & other) {
    if (this != &other) {
        disperse();
        copy_nodes_from(other.nodes_);
    }
    return *this;
}

NodeTeam & NodeTeam::operator=(NodeTeam && other) {
    if (this == &other) {
        // Take over the already allocated nodes
        nodes_ = other.nodes_;
        other.nodes_.clear();
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
}

void NodeTeam::remake(const Roulette<Module *> & mod_list) {
    disperse();

    // Pick random initial member
    const Module * prot = mod_list.draw();
    add_member(prot);
}

const Node * NodeTeam::invite_new_member(const ChainSeeker & seeker, const Link & link) {
    const Node * node_a = seeker.node;
    const Node * node_b = add_member(link.mod, node_a->tx() * link.tx);

    const TerminusType term_a = free_seekers_.get_term(seeker);
    const TerminusType term_b = OPPOSITE_TERM[term_a];

    free_seekers_.get_vm(term_a).erase(seeker); // invalidates seeker

    const ChainSeeker seeker_b = ChainSeeker(node_b, link.target_chain_id);
    free_seekers_.get_vm(term_b).erase(seeker_b);

    return node_b;
}

}  /* elfin */