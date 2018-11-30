#include "node.h"

#include <sstream>

#define ENABLE_CHECKS

namespace elfin {

Node::Node(const Module * prototype, const Transform & tx) :
    prototype_(prototype), tx_(tx) {
    for (auto mapitr : prototype_->chains()) {
        auto & chain = mapitr.second;
        const size_t chain_id = prototype_->chain_id_map().at(chain.name);

        if (chain.n_links.size() > 0) {
#ifdef ENABLE_CHECKS
            if (free_term_.n.find(chain_id) != free_term_.n.end()) {
                die(("Tried to insert chain_id[%lu] that already "
                     "exists in free_term_.n of\n\t%s\n"),
                     to_string().c_str());
            }
#endif  /* ifdef ENABLE_CHECKS */

            free_term_.n.insert(chain_id);
        }
        if (chain.c_links.size() > 0) {
#ifdef ENABLE_CHECKS
            if (free_term_.c.find(chain_id) != free_term_.c.end()) {
                die(("Tried to insert chain_id[%lu] that already "
                     "exists in free_term_.c of\n\t%s\n"),
                     to_string().c_str());
            }
#endif  /* ifdef ENABLE_CHECKS */

            free_term_.c.insert(chain_id);
        }
    }
}

void Node::occupy_terminus(TerminusType term, size_t chain_id) {
    IdSet free_chains = free_term_.get(term);
    auto itr = free_chains.find(chain_id);

#ifdef ENABLE_CHECKS
    if (itr == free_chains.end()) {
        die(("Tried to occupy Terminus[%s] on Chain[%s] "
             "but terminus is already busy.\n"
             "\t%s\n"),
            TerminusTypeNames[term],
            prototype_->chain_names().at(chain_id).c_str(),
            to_string().c_str());
    }
#endif  /* ifdef ENABLE_CHECKS */

    free_chains.erase(chain_id);
}

void Node::free_terminus(TerminusType term, size_t chain_id) {
    IdSet free_chains = free_term_.get(term);
    auto itr = free_chains.find(chain_id);

#ifdef ENABLE_CHECKS
    if (itr != free_chains.end()) {
        die(("Tried to free Terminus[%s] on Chain[%s] "
             "but terminus was not occupied in the first place.\n"
             "\t%s\n"),
            TerminusTypeNames[term],
            prototype_->chain_names().at(chain_id).c_str(),
            to_string().c_str());
    }
#endif  /* ifdef ENABLE_CHECKS */

    free_chains.insert(chain_id);
}

std::string Node::to_string() const {
    std::stringstream ss;
    ss << "Node[" << prototype_->name_ << "]" << std::endl;
    ss << "Tx: " << tx_.to_string();
    return ss.str();
}

std::string Node::to_csv_string() const {
    die("Revise this function?\n");
    return tx_.to_csv_string();
}

}  /* elfin */