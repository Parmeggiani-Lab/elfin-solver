#ifndef FREE_CANDIDATE_H_
#define FREE_CANDIDATE_H_

#include "candidate.h"

namespace elfin {

class FreeCandidate : public Candidate {
public:
    /* strings */
    virtual std::string to_string() const;

    virtual void score(const WorkArea & wa);
    virtual void mutate(
        long rank,
        const CandidateLengths & cd_lens,
        const MutationCounters & mt_counters = MutationCounters());
    virtual Candidate * new_copy() const;
    std::vector<Candidate::Node> gen_random_nodes(int gen_max_len) const;
};

}  /* elfin */

#endif  /* end of include guard: FREE_CANDIDATE_H_ */