#ifndef TERMINUS_TRACKER_H_
#define TERMINUS_TRACKER_H_

#include <vector>

#include "id_types.h"
#include "terminus_type.h"
#include "chain.h"

namespace elfin {

class TerminusTracker {
private:
    /* types */
    struct FreeChainListPair {
        IdList n, c;
        size_t size(const TerminusType term) const;
        IdList & get(const TerminusType term);
    };

    /* data members */
    FreeChainListPair free_chains_;

public:
    /* ctors & dtors */
    TerminusTracker(const ChainList & chains);

    /* getters & setters */

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