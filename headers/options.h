#ifndef OPTIONS_H_
#define OPTIONS_H_

#include <string>

namespace elfin {

typedef struct {
    bool valid = true;

    // Input settings
    std::string xdb = "xdb.json";
    std::string input_file = "";

    std::string config_file = "";
    std::string output_dir = "output";

    ulong len_dev_alw = 3;

    // Average CoM distance found by xDBStat.py as
    // of 23/April/2017 is 37.9
    float avg_pair_dist = 38.0f;

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

}  /* elfin */

#endif  /* end of include guard: OPTIONS_H_ */