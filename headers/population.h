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
    MutationCutoffs mutation_cutoffs_;
    PopulationCounters counts_;

public:

    Population(const Options & options, const WorkArea & work_area);

    /* getters */
    const std::vector<Candidate *> & candidates() const { return candidates_; }
    void resize(size_t size) { candidates_.resize(size); }
    const PopulationCounters & counts() const { return counts_; }
    const MutationCutoffs & mutation_cutoffs() const { return mutation_cutoffs_; }

    void init(size_t size, bool randomize);
    void evolve(const Population * prev_gen);
    void score();
    void rank();
    void select();
};

}  /* elfin */

#endif  /* end of include guard: POPULATION_H_ */