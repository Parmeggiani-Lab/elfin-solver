#ifndef TWO_HINGE_CANDIDATE_H_
#define TWO_HINGE_CANDIDATE_H_

#include "candidate.h"

namespace elfin {

class TwoHingeCandidate : public Candidate {
public:
    virtual void score(const WorkArea & wa) {}
    virtual void mutate(
        long rank,
        const CandidateLengths & cd_lens,
        const MutationCounters & mt_counters=MutationCounters()) {}
    virtual Candidate * new_copy() const { return nullptr; }
};

}  /* elfin */

#endif  /* end of include guard: TWO_HINGE_CANDIDATE_H_ */