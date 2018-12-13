#ifndef ELFIN_H_
#define ELFIN_H_

#include <vector>
#include <memory>

#include "evolution_solver.h"

namespace elfin {

class Elfin {
private:
    /* data */
    EvolutionSolver* solver_;
    bool solver_started_ = false;
    static std::vector<Elfin *> instances_;

    /* accessors */
    void crash_dump() const;
    int run_unit_tests() const;
    int run_meta_tests() const;

    /* handlers */
    static void interrupt_handler(int const signal);
public:
    /* ctors */
    Elfin(int const argc, const char ** argv);

    /* dtors */
    ~Elfin();

    /* modifiers */
    int run();
};

} /* elfin */

#endif /* end of include guard: ELFIN_H_ */