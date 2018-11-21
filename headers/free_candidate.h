#ifndef FREE_CANDIDATE_H_
#define FREE_CANDIDATE_H_

#include "candidate.h"

namespace elfin {

// Some of the stochastic processes may fail
// to meet algorithm criteria
#define MAX_FREECANDIDATE_MUTATE_FAILS 10

class FreeCandidate : public Candidate {
private:
    void randomize();
    void auto_mutate();
    bool cross_mutate(const FreeCandidate & father);
    bool point_mutate();
    bool limb_mutate();

public:
    /* strings */
    virtual std::string to_string() const;

    virtual void score(const WorkArea & wa);
    virtual void mutate(
        long rank,
        MutationCounters & mt_counters,
        const CandidateList & candidates);

    /* ctors & dtors */
    virtual FreeCandidate * clone() const;
    virtual ~FreeCandidate() {}
};

}  /* elfin */

#endif  /* end of include guard: FREE_CANDIDATE_H_ */