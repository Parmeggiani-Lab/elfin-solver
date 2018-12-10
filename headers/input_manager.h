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
    static InputManager & instance() {
        static InputManager im;
        return im;
    }

    /* static setup methods */
    static void setup(const int argc, const char ** argv);

    /* getters & setters */
    static const Options & options() {
        return instance().options_;
    }
    static const Cutoffs & cutoffs() {
        return instance().cutoffs_;
    }
    static const Database & xdb() {
        return instance().xdb_;
    }
    static const Spec & spec() {
        return instance().spec_;
    }

    static GATimes & ga_times() {
        return instance().ga_times_;
    }
};

extern const Options & OPTIONS;
extern const Cutoffs & CUTOFFS;
extern const Database & XDB;
extern const Spec & SPEC;

extern const GATimes & GA_TIMES;

}  /* elfin */

#endif  /* end of include guard: INPUT_MANAGER_H_ */