#include "terminus_tracker.h"

#include <algorithm>

namespace elfin {

/* private */
IdList & TerminusTracker::FreeChainListPair::_get(const TerminusType term) {
    if (term == TerminusType::N) {
        return n_;
    }
    else if (term == TerminusType::C) {
        return c_;
    }
    else {
        death_by_bad_terminus(__PRETTY_FUNCTION__, term); // Aborts
        exit(1); // To suppress warning
    }
}

/* public */
size_t TerminusTracker::FreeChainListPair::size(
    const TerminusType term) const {
    if (term == TerminusType::N) {
        return n_.size();
    }
    else if (term == TerminusType::C) {
        return c_.size();
    }
    else if (term == TerminusType::ANY) {
        return n_.size() + c_.size();
    }
    else {
        death_by_bad_terminus(__PRETTY_FUNCTION__, term); // Aborts
        exit(1); // To suppress warning
    }
}

bool TerminusTracker::is_free(const TerminusType term, const size_t chain_id) const {
    const IdList & chain_ids = get_free(term);
    auto itr = std::find(chain_ids.begin(), chain_ids.end(), chain_id);
    return itr == chain_ids.end();
}
void TerminusTracker::occupy_terminus(const TerminusType term, const size_t chain_id) {
    IdList & chain_ids = get_free(term);
    auto itr = std::find(chain_ids.begin(), chain_ids.end(), chain_id);

#ifdef NDEBUG
    if (itr == chain_ids.end()) {
        die(("Tried to occupy Terminus[%s] on Chain[%s] "
             "but terminus is already busy.\n"
             "\t%s\n"),
            TerminusTypeNames[term],
            prototype_->chain_names().at(chain_id).c_str(),
            to_string().c_str());
    }
#endif  /* ifdef NDEBUG */

    *itr = chain_ids.back();
    chain_ids.pop_back();
}

void TerminusTracker::free_terminus(const TerminusType term, const size_t chain_id) {
    auto & chain_ids = get_free(term);
    auto itr = std::find(chain_ids.begin(), chain_ids.end(), chain_id);

#ifdef NDEBUG
    if (itr != chain_ids.end()) {
        die(("Tried to free Terminus[%s] on Chain[%s] "
             "but terminus was not occupied in the first place.\n"
             "\t%s\n"),
            TerminusTypeNames[term],
            prototype_->chain_names().at(chain_id).c_str(),
            to_string().c_str());
    }
#endif  /* ifdef NDEBUG */

    chain_ids.push_back(chain_id);
}

}  /* elfin */