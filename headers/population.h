#ifndef POPULATION_H_
#define POPULATION_H_

#include <vector>
#include <type_traits>

#include "candidate.h"
#include "work_area.h"
#include "jutil.h"
#include "options.h"
#include "counter_structs.h"

namespace elfin {

class Population
{
protected:
    std::vector<Candidate *> candidates_;
    const WorkArea & work_area_;
    PopulationCounters pop_counters_;
    CandidateLengths candidate_lengths_;

    Candidate * new_candidate(bool randomize) const;

public:
    Population(const WorkArea & work_area);
    virtual ~Population();

    /* getters */
    const std::vector<Candidate *> & candidates() const { return candidates_; }
    void resize(size_t size) { candidates_.resize(size); }
    const PopulationCounters & pop_counters() const { return pop_counters_; }
    const CandidateLengths & candidate_lengths() const { return candidate_lengths_; }

    void init(size_t size, bool randomize);
    void evolve(const Population * prev_gen);
    void score();
    void rank();
    void select();
};

}  /* elfin */

#endif  /* end of include guard: POPULATION_H_ */