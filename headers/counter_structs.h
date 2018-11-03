#ifndef COUNTER_STRUCTS_H_
#define COUNTER_STRUCTS_H_

namespace elfin {

typedef struct {
    ulong cross = 0;
    ulong point = 0;
    ulong limb = 0;
} MutationCutoffs;

typedef struct {
    ulong cross = 0;
    ulong point = 0;
    ulong limb = 0;
    ulong rand = 0;
    ulong cross_fail = 0;
} MutationCounters;

typedef struct {
    ulong pop_size = 0;
    ulong survivors = 0;
    ulong non_survivors = 0;
    double evolve_time = 0.0f;
    double score_time = 0.0f;
    double rank_time = 0.0f;
    double select_time = 0.0f;
    struct {
        ulong expected = 0;
        ulong min = 0;
        ulong max = 0;
    } len;
} PopulationCounters;

}  /* elfin */

#endif  /* end of include guard: COUNTER_STRUCTS_H_ */