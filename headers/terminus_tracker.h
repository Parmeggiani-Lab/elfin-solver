#ifndef TERMINUS_TRACKER_H_
#define TERMINUS_TRACKER_H_

#include <vector>

#include "id_types.h"
#include "module.h"
#include "terminus_type.h"
#include "chain.h"

namespace elfin {

class TerminusTracker {
private:
    /* types */
    struct FreeChainListPair {
        /*
         * It's possible to provide O(1) find() and pick_random()
         */
        IdList n, c;
        size_t get_size(const TerminusType term) const;
        IdList & get(const TerminusType term);
        const IdList & get(const TerminusType term) const {
            return const_cast<FreeChainListPair *>(this)->get(term);
        }
    };

    /* data members */
    const Module * prototype_ = nullptr;
    FreeChainListPair free_chains_, busy_chains_;

    /* other methods */
    bool is_free(const TerminusType term, const size_t chain_id) const;
public:
    /* ctors & dtors */
    TerminusTracker(const Module * proto);

    /* getters & setters */

    /* other methods */
    size_t count_free_chains(const TerminusType term) const {
        return free_chains_.get_size(term);
    }
    size_t pick_random_free_chain(const TerminusType term) const;
    void occupy_terminus(const TerminusType term, const size_t chain_id);
    void free_terminus(const TerminusType term, const size_t chain_id);
};

}  /* elfin */

#endif  /* end of include guard: TERMINUS_TRACKER_H_ */