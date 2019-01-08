#include "tests.h"

#include "test_stat.h"
#include "test_data.h"

// Include test subject modules.
#include "work_area.h"
#include "proto_tests.h"
#include "scoring.h"
#include "random_utils.h"
#include "input_manager.h"
#include "path_generator.h"
#include "path_team.h"
#include "hinge_team.h"
#include "double_hinge_team.h"
#include "evolution_solver.h"

namespace elfin {

namespace tests {

TestStat test_units() {
    JUtil.info("Running unit tests...\n");
    TestStat total;

    // Only setup XDB once.
    InputManager::parse({ "elfin", "--xdb_file", "xdb.json" });
    InputManager::setup_xdb();

    total += InputManager::test();

    // Stop doing more tests if there are failures.

    if (total.errors == 0)
        total += proto::test();
    
    if (total.errors == 0)
        total += WorkArea::test();

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

    if (total.errors == 0)
        total += DoubleHingeTeam::test();

    return total;
}

TestStat test_integration() {
    JUtil.info("Running integration tests...\n");
    TestStat total = EvolutionSolver::test();

    return total;
}

void run_all() {
    auto const unit_ts = test_units();

    if (unit_ts.errors > 0) {
        PANIC("%zu unit tests failed. Not continuing to integration tests.\n",
              unit_ts.errors);
    }
    else {
        auto const inte_ts = test_integration();

        PANIC_IF(inte_ts.errors > 0,
                 "%zu integration tests failed.\n",
                 inte_ts.errors);

        JUtil.info("%zu/%zu unit tests passed.\n",
                   unit_ts.tests, unit_ts.tests);

        JUtil.info("%zu/%zu integration tests passed.\n",
                   inte_ts.tests, inte_ts.tests);

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