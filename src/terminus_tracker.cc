#include "terminus_tracker.h"

#include <algorithm>
#include <sstream>

#include "string_utils.h"

namespace elfin {

/* private */
size_t TerminusTracker::FreeChainListPair::get_size(
    const TerminusType term) const {
    if (term == TerminusType::N) {
        return n.size();
    }
    else if (term == TerminusType::C) {
        return c.size();
    }
    else if (term == TerminusType::ANY) {
        return n.size() + c.size();
    }
    else {
        death_by_bad_terminus(__PRETTY_FUNCTION__, term); // Aborts
        exit(1); // To suppress warning
    }
}

IdList & TerminusTracker::FreeChainListPair::get(
    const TerminusType term) {
    if (term == TerminusType::N) {
        return n;
    }
    else if (term == TerminusType::C) {
        return c;
    }
    else {
        death_by_bad_terminus(__PRETTY_FUNCTION__, term); // Aborts
        exit(1); // To suppress warning
    }
}

bool TerminusTracker::is_free(
    const TerminusType term,
    const size_t chain_id) const {
    const IdList & chain_ids = free_chains_.get(term);
    auto itr = std::find(chain_ids.begin(), chain_ids.end(), chain_id);
    return itr == chain_ids.end();
}

/* public */
TerminusTracker::TerminusTracker(const Module * proto) :
    prototype_(proto) {
    for (auto & chain : proto->chains()) {
        const size_t chain_id = proto->chain_id_map().at(chain.name);

        if (chain.n_term().links().size() > 0) {
            free_chains_.get(TerminusType::N).push_back(chain_id);
        }
        if (chain.c_term().links().size() > 0) {
            free_chains_.get(TerminusType::C).push_back(chain_id);
        }
    }
}

size_t TerminusTracker::pick_random_free_chain(
    const TerminusType term) const {
    const IdList & chain_ids = free_chains_.get(term);
    DEBUG(0 == chain_ids.size());
    return pick_random(chain_ids);
}

std::string TerminusTracker::to_string() const {
    std::ostringstream ss;

    ss << "Terminus[\n";
    ss << "\t Prototype: " << prototype_->name.c_str() << std::endl;

    auto print_chain_lists = [&](const IdList & chain_ids) {
        for (const size_t id : chain_ids) {
            ss << "\t\t#" << id << " : ";
            ss << prototype_->chains().at(id).name.c_str() << std::endl;
        }
    };

    ss << "\tChain free on N-term:" << std::endl;
    print_chain_lists(free_chains_.n);

    ss << "\tChain free on C-term:" << std::endl;
    print_chain_lists(free_chains_.c);

    ss << "\tChain busy on N-term:" << std::endl;
    print_chain_lists(busy_chains_.n);

    ss << "\tChain busy on C-term:" << std::endl;
    print_chain_lists(busy_chains_.c);

    return ss.str();
}

void TerminusTracker::occupy_terminus(
    const TerminusType term,
    const size_t chain_id) {
    IdList & fchain_ids = free_chains_.get(term);
    auto itr = std::find(fchain_ids.begin(), fchain_ids.end(), chain_id);

    if (itr == fchain_ids.end()) {
        die(("Tried to occupy Terminus[%s] on Chain[%s] "
             "but terminus is already busy.\n"
             "\t%s\n"),
            TerminusTypeNames[term],
            prototype_->chain_names.at(chain_id).c_str(),
            to_string().c_str());
    }

    // Move ID to busy
    busy_chains_.get(term).push_back(chain_id);

    // Delete ID from free
    *itr = fchain_ids.back();
    fchain_ids.pop_back();
}

void TerminusTracker::free_terminus(
    const TerminusType term,
    const size_t chain_id) {
    auto & bchain_ids = busy_chains_.get(term);
    auto itr = std::find(bchain_ids.begin(), bchain_ids.end(), chain_id);

    if (itr != bchain_ids.end()) {
        die(("Tried to free Terminus[%s] on Chain[%s] "
             "but terminus was not occupied in the first place.\n"
             "\t%s\n"),
            TerminusTypeNames[term],
            prototype_->chain_names.at(chain_id).c_str(),
            to_string().c_str());
    }

    // Move ID to free
    free_chains_.get(term).push_back(chain_id);

    // Delete ID from busy
    *itr = bchain_ids.back();
    bchain_ids.pop_back();
}

}  /* elfin */