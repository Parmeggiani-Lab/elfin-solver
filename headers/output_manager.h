#ifndef OUTPUT_MANAGER_H_
#define OUTPUT_MANAGER_H_

#include <string>

#include "evolution_solver.h"
#include "input_manager.h"

namespace elfin {

class OutputManager
{
public:

    static void write_output(
        EvolutionSolver const& solver,
        std::string extra_dir = "",
        size_t const indent_size = 4);
};

}  /* elfin */

#endif  /* end of include guard: OUTPUT_MANAGER_H_ */