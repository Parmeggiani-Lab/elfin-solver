#include "test_manager.h"

#include "test_stat.h"
#include "test_consts.h"

#include "kabsch.h"
#include "random_utils.h"
#include "input_manager.h"

namespace elfin {

/* private */
size_t TestManager::test_units() const {
    msg("Running unit tests...\n");
    TestStat total;

    total += InputManager::test();
    total += random::test();
    total += Transform::test();
    total += Vector3f::test();
    total += kabsch::test();

    msg("%lu/%lu unit tests passed.\n",
        (total.tests - total.errors), total.tests);
    if (total.errors > 0) {
        err("%lu unit tests failed!\n", total.errors);
    }
    return total.errors;
}

size_t TestManager::test_integration() const {
    msg("Running integration tests...\n");
    TestStat total;

    wrn("TODO: Integration tests\n");

    msg("%lu/%lu integration tests passed.\n",
        (total.tests - total.errors), total.tests);
    if (total.errors > 0) {
        err("%lu integration tests failed!\n", total.errors);
    }
    return total.errors;
}

/* public */
/* accessors */
void TestManager::run() const {
    size_t const unit_test_errors = test_units();

    if (unit_test_errors > 0) {
        die("%lu unit tests failed. Not continuing to integration tests.\n",
            unit_test_errors);
    }
    else {
        size_t const int_test_errors = test_integration();

        if (int_test_errors > 0) {
            die("%lu integration tests failed.\n",
                int_test_errors);
        } else {
            msg("All Tests Passed. \\*O*/\n");
            raw(unit_tests_passed_str);
        }
    }
}

}  /* elfin */