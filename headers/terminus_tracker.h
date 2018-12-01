#ifndef TERMINUS_TRACKER_H_
#define TERMINUS_TRACKER_H_

#include <vector>

#include "terminus_type.h"

namespace elfin {

class TerminusTracker {
private:
    /* data members */
    /*
     * n and c store chain ids (convertible to chain names). Although
     * vectors require O(n) for find() in occupy_terminus, n are c are
     * usually very small vectors.
     *
     * Other choices are:
     * - use unordered_set for O(1) find() but O(n)
     * pick_random().
     * - use unordered_set with a vector of pointers into the set. O(1)
     *   for all but requires extra data.
    */
    std::vector<size_t> & __get(TerminusType term) {
        if (term == TerminusType::N) {
            return n_;
        }
        else if (term == TerminusType::C) {
            return c_;
        }
        else {
            death_by_bad_terminus(__PRETTY_FUNCTION__, term);
        }
    }
public:
    std::vector<size_t> n_, c_;
    std::vector<size_t> & get(TerminusType term) {
        return __get(term);
    }
    const std::vector<size_t> & get(TerminusType term) const {
        return const_cast<TerminusTracker *>(this)->__get(term);
    }
    size_t size(TerminusType term) const {
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
            death_by_bad_terminus(__PRETTY_FUNCTION__, term);
        }
    }
};

}  /* elfin */

#endif  /* end of include guard: TERMINUS_TRACKER_H_ */