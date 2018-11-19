#ifndef TWO_HINGE_CANDIDATE_H_
#define TWO_HINGE_CANDIDATE_H_

#include "candidate.h"

namespace elfin {

class TwoHingeCandidate : public Candidate {
public:
    virtual void score(const WorkArea & wa) {}
    virtual void mutate(
        long rank,
        MutationCounters & mt_counters,
        const CandidateList & candidates) {}
    virtual TwoHingeCandidate * clone() const { return nullptr; }
};

}  /* elfin */

#endif  /* end of include guard: TWO_HINGE_CANDIDATE_H_ */