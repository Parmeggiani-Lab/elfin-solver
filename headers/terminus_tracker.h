#ifndef TERMINUS_TRACKER_H_
#define TERMINUS_TRACKER_H_

#include <vector>

#include "id_types.h"
#include "terminus_type.h"

namespace elfin {

class TerminusTracker {
private:
    /* types */
    class FreeChainListPair {
    private:
        /*
         * n and c store chain ids (convertible to chain names). Although
         * vectors require O(n) for find(), n are c are usually very small
         * vectors.
         *
         * Other choices are:
         * - use unordered_set for O(1) find() but O(n)
         * pick_random().
         * - use unordered_set with a vector of pointers into the set. O(1)
         *   for all but requires extra data.
        */
        IdList n_, c_;
        IdList & _get(const TerminusType term);
    public:
        IdList & get(const TerminusType term) { return _get(term); }
        const IdList & get(const TerminusType term) const {
            return const_cast<TerminusTracker::FreeChainListPair *>(this)->_get(term);
        }
        size_t size(const TerminusType term) const;
    };

    /* data members */
    FreeChainListPair free_chains_;

public:
    /* getters & setters */
    IdList & get_free(const TerminusType term) { return free_chains_.get(term); }
    const IdList & get_free(const TerminusType term) const {
        return free_chains_.get(term);
    }

    /* other methods */
    size_t get_free_size(const TerminusType term) const {
        return free_chains_.size(term);
    }
    bool is_free(const TerminusType term, const size_t chain_id) const;
    void occupy_terminus(const TerminusType term, const size_t chain_id);
    void free_terminus(const TerminusType term, const size_t chain_id);
};

}  /* elfin */

#endif  /* end of include guard: TERMINUS_TRACKER_H_ */