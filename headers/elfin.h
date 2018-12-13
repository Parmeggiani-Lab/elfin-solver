#ifndef ELFIN_H_
#define ELFIN_H_

#include <vector>
#include <memory>

#include "evolution_solver.h"

namespace elfin {

class ElfinRunner
{
private:
    EvolutionSolver* solver_;

    bool es_started_ = false;
    static std::vector<ElfinRunner *> instances_;

    void crash_dump() const;
    int run_unit_tests() const;
    int run_meta_tests() const;
    static void interrupt_handler(int const signal);
public:
    ElfinRunner(int const argc, const char ** argv);
    void run();
};

} /* elfin */

#endif /* end of include guard: ELFIN_H_ */