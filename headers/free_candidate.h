#ifndef FREE_CANDIDATE_H_
#define FREE_CANDIDATE_H_

#include "candidate.h"
#include "chain_seeker.h"
#include "basic_node_team.h"

namespace elfin {

// Some of the stochastic processes may fail to meet algorithm criteria. We
// need to limit the number of tries.
#define MAX_FREECANDIDATE_MUTATE_FAILS 10

class FreeCandidate : public Candidate {
private:
    virtual bool cross_mutate(
        const Candidate * mother,
        const Candidate * father);
    virtual bool point_mutate();
    virtual bool limb_mutate();
    virtual void grow(ChainSeeker seeker);
    virtual void regrow();
    bool pick_sever_point(
        size_t & id_out,
        const TerminusType term = TerminusType::ANY);

public:
    /* strings */
    virtual std::string to_string() const;

    /* ctors */
    FreeCandidate() : Candidate(new BasicNodeTeam()) {}
    FreeCandidate(const FreeCandidate & other) : Candidate(other) {}
    FreeCandidate(FreeCandidate && other) : Candidate(other) {}
    virtual FreeCandidate * clone() const;

    /* dtors */
    virtual ~FreeCandidate() {}

    /* modifiers */
    virtual void score(const WorkArea & wa);

    /* printers */
};

}  /* elfin */

#endif  /* end of include guard: FREE_CANDIDATE_H_ */