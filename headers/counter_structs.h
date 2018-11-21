#ifndef COUNTER_STRUCTS_H_
#define COUNTER_STRUCTS_H_

namespace elfin {

struct MutationCounters {
    size_t cross = 0;
    size_t cross_fail = 0;
    size_t point = 0;
    size_t point_fail = 0;
    size_t limb = 0;
    size_t limb_fail = 0;
    size_t rand = 0;
};

extern const MutationCounters & MUTATION_COUNTERS; // defined in population.cc

struct PopulationCounters {
    size_t pop_size = 0;
    size_t survivors = 0;
    size_t non_survivors = 0;
    double evolve_time = 0.0f;
    double score_time = 0.0f;
    double rank_time = 0.0f;
    double select_time = 0.0f;
};

extern const PopulationCounters & POPULATION_COUNTERS; // defined in population.cc

}  /* elfin */

#endif  /* end of include guard: COUNTER_STRUCTS_H_ */