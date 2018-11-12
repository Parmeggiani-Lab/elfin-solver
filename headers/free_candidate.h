#ifndef FREE_CANDIDATE_H_
#define FREE_CANDIDATE_H_

#include "candidate.h"

namespace elfin {

// Some of the stochastic processes may fail
// to meet algorithm criteria
#define MAX_FREECANDIDATE_MUTATE_FAILS 10

class FreeCandidate : public Candidate {
private:
    static void gen_random_nodes(
        Nodes & nodes,
        const size_t & max_len = CANDIDATE_LENGTHS.max);
    static void gen_random_nodes_reverse(
        Nodes & nodes,
        const size_t & max_len = CANDIDATE_LENGTHS.max);
    static bool synthesise(Nodes & nodes);
    static bool synthesise_reverse(Nodes & nodes);

    void randomize();
    void auto_mutate();
    bool cross_mutate(
        const FreeCandidate & father,
        FreeCandidate & out) const;
    bool point_mutate();
    bool get_severable_id(
        size_t & sever_id,
        bool & mutate_left_limb) const;
    bool limb_mutate();

public:
    /* strings */
    virtual std::string to_string() const;

    virtual void score(const WorkArea & wa);
    virtual void mutate(
        long rank,
        MutationCounters & mt_counters,
        const Candidates & candidates);

    /* ctors & dtors */
    virtual FreeCandidate * clone() const;
    virtual ~FreeCandidate() {}
};

}  /* elfin */

#endif  /* end of include guard: FREE_CANDIDATE_H_ */