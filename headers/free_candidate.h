#ifndef FREE_CANDIDATE_H_
#define FREE_CANDIDATE_H_

#include "candidate.h"

namespace elfin {

class FreeCandidate : public Candidate {
public:
    FreeCandidate() {}

    /* strings */
    virtual std::string to_string(const IdNameMap & inm) const;

    virtual void init(const WorkArea & wa);
    virtual void score(const WorkArea & wa);
    virtual void mutate(size_t rank);
};

}  /* elfin */

#endif  /* end of include guard: FREE_CANDIDATE_H_ */