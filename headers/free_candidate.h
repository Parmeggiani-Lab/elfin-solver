#ifndef FREE_CANDIDATE_H_
#define FREE_CANDIDATE_H_

#include "candidate.h"
#include "basic_node_team.h"

namespace elfin {

// Some of the stochastic processes may fail to meet algorithm criteria. We
// need to limit the number of tries.
#define MAX_FREECANDIDATE_MUTATE_FAILS 10

class FreeCandidate : public Candidate {
public:
    /* ctors */
    FreeCandidate() : Candidate(new BasicNodeTeam()) {}
    virtual FreeCandidate * clone() const;

    /* dtors */
    virtual ~FreeCandidate() {}

    /* accessors */
    virtual Crc32 checksum() const {
        return node_team_->checksum();
    }

    /* modifiers */
    virtual void score(const WorkArea * wa) {
        score_ = node_team_->score(wa);
    }

    /* printers */
    virtual std::string to_string() const;
};

}  /* elfin */

#endif  /* end of include guard: FREE_CANDIDATE_H_ */