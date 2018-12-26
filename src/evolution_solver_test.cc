#include "evolution_solver.h"

#include "test_stat.h"
#include "input_manager.h"

namespace elfin {

TestStat EvolutionSolver::test() {
    TestStat ts;

    // Test solving free body type work area.
    InputManager::load_test_config(
        /*spec_file=*/ "examples/quarter_snake_free_far.json");

    EvolutionSolver es;

    // Test solver runs without crashing.
    {
        ts.tests++;

        LogLvl original_ll = ::get_log_level();
        
        ::set_log_level(LOG_RAW);
        es.run();
        ::set_log_level(original_ll);
    }

    // Test that expected solution was obtained.
    {
        ts.tests++;
        ts.errors++;
    }

    return ts;
}

}  /* elfin */