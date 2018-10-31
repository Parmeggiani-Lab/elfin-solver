#ifndef POPULATION_H_
#define POPULATION_H_

#include <vector>
#include <type_traits>

#include "candidate.h"
#include "work_area.h"
#include "jutil.h"
#include "options.h"

namespace elfin {

class Population
{
protected:
    std::vector<Candidate *> candidates_;
    const WorkArea & work_area_;

    ulong non_surviver_count_;
    ulong surviver_cutoff_;

public:;
    Population(const Options & options, const WorkArea & work_area);

    /* getters */
    const std::vector<Candidate *> & candidates() const { return candidates_; }
    void resize(size_t size) { candidates_.resize(size); }

    void init(size_t size, bool randomize);
    void evolve(const Population * prev_gen);
    void score();
    void rank();
    void select();
};

}  /* elfin */

#endif  /* end of include guard: POPULATION_H_ */