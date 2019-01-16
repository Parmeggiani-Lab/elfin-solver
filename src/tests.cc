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

    auto const test_fragment = [&total](TestStat (*test_func)(void)) {
        // Stop doing more tests if there are failures.
        if (total.errors == 0)
            total += test_func();
    };

    test_fragment(InputManager::test);

    test_fragment(proto::test);
    test_fragment(WorkArea::test);
    test_fragment(random::test);
    test_fragment(Transform::test);
    test_fragment(Vector3f::test);
    test_fragment(scoring::test);

    test_fragment(PathTeam::test);
    test_fragment(PathGenerator::test);
    test_fragment(HingeTeam::test);
    test_fragment(DoubleHingeTeam::test);
    return total;
}

TestStat test_integration() {
    JUtil.info("Running integration tests...\n");
    TestStat total;

    auto const test_fragment = [&total](TestStat (*test_func)(void)) {
        // Stop doing more tests if there are failures.
        if (total.errors == 0)
            total += test_func();
    };

    test_fragment(EvolutionSolver::test);

    return total;
}

void run_all() {
    auto const unit_ts = test_units();

    if (unit_ts.errors > 0) {
        auto const& msg =
            std::to_string(unit_ts.errors) + " unit tests failed. " +
            "Not continuing to integration tests.\n";
        throw ExitException(1, msg);
    }
    else {
        auto const inte_ts = test_integration();

        PANIC_IF(inte_ts.errors > 0,
                 BadArgument(std::to_string(inte_ts.errors) +
                             " integration tests failed.\n"));

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