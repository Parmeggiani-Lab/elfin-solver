#ifndef FREE_CANDIDATE_H_
#define FREE_CANDIDATE_H_

#include "candidate.h"

namespace elfin {

// Some of the stochastic processes may fail
// to meet algorithm criteria
#define MAX_FREECANDIDATE_MUTATE_FAILS 10

class FreeCandidate : public Candidate {
private:
    static Nodes gen_random_nodes(size_t max_len, Nodes & nodes);
    static Nodes gen_random_nodes_reverse(size_t max_len, Nodes & nodes);
    static bool synthesise(Nodes & nodes);
    static bool synthesise_reverse(Nodes & nodes);

    void randomize();
    void auto_mutate();
    bool cross_mutate(
        const FreeCandidate & father,
        FreeCandidate & out) const;
    bool point_mutate();
    bool limb_mutate();

    FreeCandidate(const Nodes & nodes) {
        nodes_ = Nodes(nodes);
    };

public:
    FreeCandidate() {};

    /* strings */
    virtual std::string to_string() const;

    virtual void score(const WorkArea & wa);
    virtual void mutate(
        long rank,
        MutationCounters & mt_counters,
        const Candidates & candidates);
    virtual Candidate * new_copy() const;
};

}  /* elfin */

#endif  /* end of include guard: FREE_CANDIDATE_H_ */