#include "tests.h"

#include "test_stat.h"
#include "test_data.h"

// Include test subject modules.
#include "scoring.h"
#include "random_utils.h"
#include "input_manager.h"
#include "path_generator.h"
#include "path_team.h"
#include "hinge_team.h"
#include "evolution_solver.h"

namespace elfin {

namespace tests {

size_t test_units() {
    JUtil.info("Running unit tests...\n");
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
        total += scoring::test();


    if (total.errors == 0)
        total += PathTeam::test();

    if (total.errors == 0)
        total += PathGenerator::test();

    if (total.errors == 0)
        total += HingeTeam::test();

    JUtil.info("%zu/%zu unit tests passed.\n",
               (total.tests - total.errors), total.tests);

    return total.errors;
}

size_t test_integration() {
    JUtil.info("Running integration tests...\n");
    TestStat total = EvolutionSolver::test();

    JUtil.info("%zu/%zu integration tests passed.\n",
               (total.tests - total.errors), total.tests);

    return total.errors;
}

void run_all() {
    size_t const unit_test_errors = test_units();

    if (unit_test_errors > 0) {
        PANIC("%zu unit tests failed. Not continuing to integration tests.\n",
              unit_test_errors);
    }
    else {
        size_t const int_test_errors = test_integration();

        PANIC_IF(int_test_errors > 0,
                 "%zu integration tests failed.\n",
                 int_test_errors);

        if (JUtil.check_log_lvl(LOGLVL_INFO)) {
            printf("- - - - - - - - - -"
                   "All Tests Passed \\*O*/"
                   "- - - - - - - - - -\n");
            printf(unit_tests_passed_str);
        }
    }
}

}  /* tests */

}  /* elfin */