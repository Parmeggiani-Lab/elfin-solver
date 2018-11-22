#ifndef OPTIONS_H_
#define OPTIONS_H_

#include <string>

namespace elfin {

struct Options {
    bool valid = true;

    std::string xdb = "xdb.json";
    std::string input_file = "";
    std::string config_file = "";
    std::string output_dir = "output";
    std::string radius_type = "max_ca_dist";

    size_t len_dev_alw = 3;

    // Run elfinpy/stat_xdb.py to find this number with the latest xdb.json
    float avg_pair_dist = 37.709864956942226f;

    /* GA parameters */
    uint rand_seed = 0x1337cafe;
    size_t ga_pop_size = 10000;
    size_t ga_iters = 1000;
    float ga_survive_rate = 0.05f;
    float ga_cross_rate = 0.5f;
    float ga_point_mutate_rate = 0.3f;
    float ga_limb_mutate_rate = 0.3f;

    // Use a small number but not exactly 0.0 because of imprecise float
    // comparison
    float ga_stop_score = 0.001f;

    long ga_stop_stagnancy = 50;

    bool run_unit_tests = false;

    int device = 0;
    size_t keep_n = 3;

    bool dry_run = false;
};

}  /* elfin */

#endif  /* end of include guard: OPTIONS_H_ */