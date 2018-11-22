#ifndef MUTATION_COUNTERS_H_
#define MUTATION_COUNTERS_H_

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

}  /* elfin */

#endif  /* end of include guard: MUTATION_COUNTERS_H_ */