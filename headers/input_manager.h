#ifndef INPUT_MANAGER_H_
#define INPUT_MANAGER_H_

#include "options.h"
#include "spec.h"
#include "database.h"

namespace elfin {

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

class InputManager
{
protected:
    Options options_;
    Cutoffs cutoffs_;
    Database xdb_;
    Spec spec_;

    GATimes ga_times_;

    InputManager() {}

    static void setup_cutoffs();
public:
    static InputManager& instance() {
        static InputManager im;
        return im;
    }

    /* static setup methods */
    static void setup(int const argc, const char ** argv);

    /* getters& setters */
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
};

extern Options const& OPTIONS;
extern Cutoffs const& CUTOFFS;
extern Database const& XDB;
extern Spec const& SPEC;

extern GATimes const& GA_TIMES;

}  /* elfin */

#endif  /* end of include guard: INPUT_MANAGER_H_ */