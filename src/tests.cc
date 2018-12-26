#include "tests.h"

#include "test_stat.h"
#include "test_data.h"

// Test subjects.
#include "kabsch.h"
#include "random_utils.h"
#include "input_manager.h"
#include "basic_node_team.h"
#include "evolution_solver.h"

namespace elfin {

namespace tests {

size_t test_units() {
    msg("Running unit tests...\n");
    TestStat total;

    total += InputManager::test();

    // Stop doing more tests if there are failures.
    if (total.errors == 0)
        total += random::test();

    if (total.errors == 0)
        total += Transform::test();

    if (total.errors == 0)
        total += Vector3f::test();

    if (total.errors == 0)
        total += kabsch::test();

    if (total.errors == 0)
        total += BasicNodeTeam::test();

    msg("%zu/%zu unit tests passed.\n",
        (total.tests - total.errors), total.tests);

    if (total.errors > 0) {
        err("%zu unit tests failed!\n", total.errors);
    }

    return total.errors;
}

size_t test_integration() {
    msg("Running integration tests...\n");
    TestStat total = EvolutionSolver::test();

    msg("%zu/%zu integration tests passed.\n",
        (total.tests - total.errors), total.tests);
    
    if (total.errors > 0) {
        err("%zu integration tests failed!\n", total.errors);
    }

    return total.errors;
}

void run_all() {
    size_t const unit_test_errors = test_units();

    if (unit_test_errors > 0) {
        die("%zu unit tests failed. Not continuing to integration tests.\n",
            unit_test_errors);
    }
    else {
        size_t const int_test_errors = test_integration();

        if (int_test_errors > 0) {
            die("%zu integration tests failed.\n",
                int_test_errors);
        } else {
            msg("All Tests Passed. \\*O*/\n");
            raw(unit_tests_passed_str);
        }
    }
}

}  /* tests */

}  /* elfin */