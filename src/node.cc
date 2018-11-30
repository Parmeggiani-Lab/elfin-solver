#include "node.h"

#include <sstream>
#include <algorithm>

namespace elfin {

Node::Node(const Module * prototype, const Transform & tx) :
    prototype_(prototype), tx_(tx) {
    for (auto & chain : prototype_->chains()) {
        const size_t chain_id = prototype_->chain_id_map().at(chain.name);

        if (chain.n_links.size() > 0) {
#ifdef NDEBUG
            auto itr = std::find(free_term_.n_.begin(), free_term_.n_.end(), chain_id);
            if (itr != free_term_.n_.end()) {
                die(("Tried to add chain_id[%lu] that already "
                     "exists in free_term_.n of\n\t%s\n"),
                    to_string().c_str());
            }
#endif  /* ifdef NDEBUG */

            free_term_.n_.push_back(chain_id);
        }
        if (chain.c_links.size() > 0) {
#ifdef NDEBUG
            auto itr = std::find(free_term_.c_.begin(), free_term_.c_.end(), chain_id);
            if (itr != free_term_.c_.end()) {
                die(("Tried to add chain_id[%lu] that already "
                     "exists in free_term_.c of\n\t%s\n"),
                    to_string().c_str());
            }
#endif  /* ifdef NDEBUG */

            free_term_.c_.push_back(chain_id);
        }
    }
}

void Node::occupy_terminus(TerminusType term, size_t chain_id) {
    std::vector<size_t> free_chains = free_term_.get(term);
    auto itr = std::find(free_chains.begin(), free_chains.end(), chain_id);

#ifdef NDEBUG
    if (itr == free_chains.end()) {
        die(("Tried to occupy Terminus[%s] on Chain[%s] "
             "but terminus is already busy.\n"
             "\t%s\n"),
            TerminusTypeNames[term],
            prototype_->chain_names().at(chain_id).c_str(),
            to_string().c_str());
    }
#endif  /* ifdef NDEBUG */

    *itr = free_chains.back();
    free_chains.pop_back();
}

void Node::free_terminus(TerminusType term, size_t chain_id) {
    std::vector<size_t> free_chains = free_term_.get(term);
    auto itr = std::find(free_chains.begin(), free_chains.end(), chain_id);

#ifdef NDEBUG
    if (itr != free_chains.end()) {
        die(("Tried to free Terminus[%s] on Chain[%s] "
             "but terminus was not occupied in the first place.\n"
             "\t%s\n"),
            TerminusTypeNames[term],
            prototype_->chain_names().at(chain_id).c_str(),
            to_string().c_str());
    }
#endif  /* ifdef NDEBUG */

    free_chains.push_back(chain_id);
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