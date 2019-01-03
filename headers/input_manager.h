#ifndef INPUT_MANAGER_H_
#define INPUT_MANAGER_H_

#include <vector>

#include "options.h"
#include "spec.h"
#include "database.h"

namespace elfin {

struct TestStat;

struct Cutoffs {
    size_t pop_size = 0;
    size_t survivors = 0;
    size_t non_survivors = 0;
};

struct GATimes {
    double evolve_time = 0.0f;
    double score_time = 0.0f;
    double rank_time = 0.0f;
    double select_time = 0.0f;
};

class InputManager {
protected:
    /* types */
    typedef std::vector<std::string> Args;

    /* data */
    Options options_;
    Cutoffs cutoffs_;
    Database xdb_;
    Spec spec_;

    GATimes ga_times_;

    /* ctors */
    InputManager() {}

    /* modifiers */
    static void setup_cutoffs();
public:
    /* data */
    static Args const test_args;

    /* accessors */
    static InputManager& instance() {
        static InputManager im;
        return im;
    }

    static Options const& options() {
        return instance().options_;
    }
    static Cutoffs const& cutoffs() {
        return instance().cutoffs_;
    }
    static Database const& xdb() {
        return instance().xdb_;
    }
    static Spec const& spec() {
        return instance().spec_;
    }

    static GATimes& ga_times() {
        return instance().ga_times_;
    }

    /* modifiers */
    static void parse_options(int const argc, char const** argv);
    static void setup();

    /* tests */
    static void load_test_config(
        std::string const& spec_file,
        size_t const n_workers = 1);
    static TestStat test();
};

extern Options const& OPTIONS;
extern Cutoffs const& CUTOFFS;
extern Database const& XDB;
extern Spec const& SPEC;

extern GATimes const& GA_TIMES;

}  /* elfin */

#endif  /* end of include guard: INPUT_MANAGER_H_ */