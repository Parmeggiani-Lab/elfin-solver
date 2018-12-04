#include "terminus_tracker.h"

#include <algorithm>

namespace elfin {

/* private */
size_t TerminusTracker::FreeChainListPair::size(
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
    const IdList & chain_ids = get_free(term);
    auto itr = std::find(chain_ids.begin(), chain_ids.end(), chain_id);
    return itr == chain_ids.end();
}

/* public */
TerminusTracker::TerminusTracker(const ChainList & chains) :
    chains_(chains) {
    for (auto & chain : prototype_->chains()) {
        const size_t chain_id = prototype_->chain_id_map().at(chain.name);

        if (chain.n_term().links().size() > 0) {
            term_tracker_.get_free(TerminusType::N).push_back(chain_id);
        }
        if (chain.c_term().links().size() > 0) {
            term_tracker_.get_free(TerminusType::C).push_back(chain_id);
        }
    }
}

const Chain & TerminusTracker::pick_random_free_chain(
    const TerminusType term) const {
    const IdList & chain_ids = free_chains_.get(term);

#ifndef NDEBUG
    DEBUG(0 == chain_ids.size());
#endif  /* ifndef NDEBUG */

    const size_t chain_id = pick_random(chain_ids);
    return chains_.at(chain_id);
}

void TerminusTracker::occupy_terminus(
    const TerminusType term,
    const size_t chain_id) {
    IdList & fchain_ids = free_chains_.get(term);
    auto itr = std::find(fchain_ids.begin(), fchain_ids.end(), chain_id);

#ifdef NDEBUG
    if (itr == fchain_ids.end()) {
        die(("Tried to occupy Terminus[%s] on Chain[%s] "
             "but terminus is already busy.\n"
             "\t%s\n"),
            TerminusTypeNames[term],
            prototype_->chain_names().at(chain_id).c_str(),
            to_string().c_str());
    }
#endif  /* ifdef NDEBUG */

    // Move ID to busy
    busy_chains_.push_back(chain_id);

    // Delete ID from free
    *itr = fchain_ids.back();
    fchain_ids.pop_back();
}

void TerminusTracker::free_terminus(
    const TerminusType term,
    const size_t chain_id) {
    auto & bchain_ids = busy_chains_.get(term);
    auto itr = std::find(bchain_ids.begin(), bchain_ids.end(), chain_id);

#ifdef NDEBUG
    if (itr != bchain_ids.end()) {
        die(("Tried to free Terminus[%s] on Chain[%s] "
             "but terminus was not occupied in the first place.\n"
             "\t%s\n"),
            TerminusTypeNames[term],
            prototype_->chain_names().at(chain_id).c_str(),
            to_string().c_str());
    }
#endif  /* ifdef NDEBUG */

    // Move ID to free
    free_chains_.push_back(chain_id);

    // Delete ID from busy
    *itr = bchain_ids.back();
    bchain_ids.pop_back();
}

}  /* elfin */