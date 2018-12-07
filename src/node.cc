#include "node.h"

#include <sstream>
#include <algorithm>

namespace elfin {

/* public */
Node::Node(const ProtoModule * prototype, const Transform & tx) :
    prototype_(prototype),
    tx_(tx) {
    // Reserve memory for maximum occupancy
    n_neighbors_.reserve(prototype_->counts().n_interfaces);
    c_neighbors_.reserve(prototype_->counts().c_interfaces);
}

void Node::store_seeker(
    const ChainSeeker & seeker,
    const TerminusType term) {
    if (term == TerminusType::N) {
        n_neighbors_.emplace_back(seeker);
        DEBUG(n_neighbors_.size() > prototype_->counts().n_interfaces,
              string_format("Too many seekers: %lu; proto module max: %lu\n",
                            n_neighbors_.size(),
                            prototype_->counts().n_interfaces));
    }
    else if (term == TerminusType::C) {
        c_neighbors_.emplace_back(seeker);
        DEBUG(c_neighbors_.size() > prototype_->counts().c_interfaces,
              string_format("Too many seekers: %lu; proto module max: %lu\n",
                            c_neighbors_.size(),
                            prototype_->counts().c_interfaces));
    }
    else {
        death_by_bad_terminus(__PRETTY_FUNCTION__, term);
    }
}

std::string Node::to_string() const {
    std::stringstream ss;
    ss << "Node[" << prototype_->name << "]" << std::endl;
    ss << "Tx: " << tx_.to_string();
    return ss.str();
}

std::string Node::to_csv_string() const {
    die("Revise this function?\n");
    return tx_.to_csv_string();
}

}  /* elfin */