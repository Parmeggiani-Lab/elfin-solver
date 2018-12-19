#include "test_manager.h"

#include "kabsch.h"
#include "random_utils.h"
#include "input_manager.h"
#include "test_stat.h"

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

    // Test solution -> kabsch -> score 0


//     for (auto itr : SPEC.work_areas()) {
//         WorkArea const& wa = itr.second;
//         V3fList moved_spec = wa.to_points();

//         // It seems that Kabsch cannot handle very large
//         // translations, but in the scale we're working
//         // at it should rarely, if ever, go beyond
//         // one thousand Angstroms
//         float const tx_ele[4][4] = {
//             1.0f, .0f, .0f, -39.0f,
//             .0f, -0.5177697998f, 0.855519979f, 999.3413f,
//             .0f, -0.855519979f, -0.5177697998f, -400.11f,
//             .0f, .0f, .0f, 1.0f
//         };

//         Transform tx(tx_ele);

//         for (Vector3f &p : moved_spec) {
//             p = tx * p;
//         }

//         // Test scoring a transformed version of spec
//         float const trx_score = kabsch::score(moved_spec, wa.to_points());
//         total_tests++;
//         if (!float_approximates(trx_score, 0)) {
//             total_errors++;
//             wrn("Self score test failed: self score should be 0\n");
//         }

//         // Test randomiser
//         int const N = 10;
//         int const rand_trials = 50000000;
//         int const expect_avg = rand_trials / N;
//         float const rand_dev_tolerance = 0.05f * expect_avg; // 5% deviation

//         int rand_count[N] = {0};
//         for (size_t i = 0; i < rand_trials; i++) {
//             size_t const dice = random::get_dice(N);
//             total_tests++;
//             if (dice >= N) {
//                 total_errors++;
//                 err("Failed to produce correct dice: random::get_dice() "
//                     "produced %lu for [0-%lu)",
//                     dice, N);
//                 break;
//             }
//             rand_count[dice]++;
//         }

//         for (size_t i = 0; i < N; i++) {
//             float const rand_dev = static_cast<float>(
//                                        abs(rand_count[i] - expect_avg) / (expect_avg));
//             total_tests++;
//             if (rand_dev > rand_dev_tolerance) {
//                 total_errors++;
//                 err("Too much random deviation: %.3f%% (expecting %d)\n",
//                     rand_dev, expect_avg);
//             }
//         }
//     }

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
    size_t const fail_count =
        test_units() +
        test_integration();

    if (fail_count > 0) {
        die("%lu unit tests failed.\n",
            fail_count);
    } else {
        msg("All Tests Passed.\n");
        raw(unit_tests_passed_str);
    }
}

}  /* elfin */