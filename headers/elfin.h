#ifndef ELFIN_H_
#define ELFIN_H_

#include <unordered_set>
#include <memory>

#include "evolution_solver.h"

namespace elfin {

class Elfin {
public: 
    /* types */
    typedef std::unordered_set<Elfin const*> InstanceMap;

    /* ctors */
    Elfin(int const argc, const char ** argv);

    /* dtors */
    ~Elfin();

    /* modifiers */
    int run();

private:
    /* data */
    EvolutionSolver solver_;
    static InstanceMap instances_;

    /* accessors */
    void crash_dump() const;

    /* handlers */
    static void interrupt_handler(int const signal);
};

} /* elfin */

#endif /* end of include guard: ELFIN_H_ */