#ifndef POPULATION_H_
#define POPULATION_H_

#include <vector>
#include <type_traits>

#include "candidate.h"
#include "work_area.h"

namespace elfin {

class Population
{
protected:
    CandidateList candidates_;
    const WorkArea & work_area_;

    struct {
        double evolve_time = 0.0f;
        double score_time = 0.0f;
        double rank_time = 0.0f;
        double select_time = 0.0f;
    } ga_times_;

    Candidate * new_candidate() const;

public:
    Population(const WorkArea & work_area);
    Population(const Population & other);
    virtual ~Population();

    /* getters */
    const CandidateList & candidates() const { return candidates_; }
    void resize(size_t size) { candidates_.resize(size); }

    void evolve(const Population * prev_gen);
    void score();
    void rank();
    void select();

    static void setup(const WorkArea & wa);
};

}  /* elfin */

#endif  /* end of include guard: POPULATION_H_ */