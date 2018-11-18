#ifndef ONE_HINGE_CANDIDATE_H_
#define ONE_HINGE_CANDIDATE_H_

#include "candidate.h"

namespace elfin {

class OneHingeCandidate : public Candidate {
public:
    virtual void score(const WorkArea & wa) {}
    virtual void mutate(
        long rank,
        MutationCounters & mt_counters,
        const Candidates & candidates) {}
    virtual OneHingeCandidate * clone() const { return nullptr; }
};

}  /* elfin */

#endif  /* end of include guard: ONE_HINGE_CANDIDATE_H_ */