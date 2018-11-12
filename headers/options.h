#ifndef OPTIONS_H_
#define OPTIONS_H_

#include <string>

namespace elfin {

/* This file declares the global structs that hold data parsed from the
configuration file and data derived from those parsed.*/

typedef struct {
    bool valid = true;

    // Input settings
    std::string xdb = "xdb.json";
    std::string input_file = "";

    std::string config_file = "";
    std::string output_dir = "output";

    ulong len_dev_alw = 3;

    // Run elfinpy/stat_xdb.py to find this number with the latest xdb.json
    float avg_pair_dist = 37.709864956942226f;

    // GA parameters
    uint rand_seed = 0x1337cafe;
    ulong ga_pop_size = 10000;
    ulong ga_iters = 1000;
    float ga_survive_rate = 0.1f;
    float ga_cross_rate = 0.5f;
    float ga_point_mutate_rate = 0.5f;
    float ga_limb_mutate_rate = 0.5f;

    // Use a small number but not exactly 0.0
    // because of imprecise float comparison
    float score_stop_threshold = 0.01f;

    int max_stagnant_gens = 50;

    bool run_unit_tests = false;

    int device = 0;
    int n_best_sols = 3;

    bool dry_run = false;
} Options;

extern const Options & OPTIONS; // defined in elfin.cc

typedef struct {
    ulong cross = 0;
    ulong point = 0;
    ulong limb = 0;
} MutationCutoffs;

extern const MutationCutoffs & MUTATION_CUTOFFS; // defined in population.cc

}  /* elfin */

#endif  /* end of include guard: OPTIONS_H_ */