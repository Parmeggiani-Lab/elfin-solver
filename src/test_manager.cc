#include "test_manager.h"

#include "kabsch.h"
#include "input_manager.h"

namespace elfin {

/* private */
size_t TestManager::test_units() const {
    msg("Running unit tests...\n");
    size_t total_errors = 0;
    size_t total_tests = 0;

    auto collect_errors = [&](
    void (*test_func)(size_t&, size_t&)) {
        size_t errors = 0, tests = 0;
        test_func(errors, tests);
        total_errors += errors;
        total_tests += tests;
    };

    collect_errors(_test_kabsch);

    msg("%lu/%lu unit tests passed.\n",
        (total_tests - total_errors), total_tests);
    return total_errors;
}

size_t TestManager::test_integration() const {
    msg("Running integration tests...\n");
    size_t total_errors = 0;
    size_t total_tests = 0;

    for (auto& itr : SPEC.work_area_map()) {
        const WorkArea& wa = itr.second;
        V3fList moved_spec = wa.to_points();

        // It seems that Kabsch cannot handle very large
        // translations, but in the scale we're working
        // at it should rarely, if ever, go beyond
        // one thousand Angstroms
        const float tx_ele[4][4] = {
            1.0f, .0f, .0f, -39.0f,
            .0f, -0.5177697998f, 0.855519979f, 999.3413f,
            .0f, -0.855519979f, -0.5177697998f, -400.11f,
            .0f, .0f, .0f, 1.0f
        };

        Transform tx(tx_ele);

        for (Vector3f &p : moved_spec) {
            p = tx * p;
        }

        // Test scoring a transformed version of spec
        const float trx_score = kabsch_score(moved_spec, wa.to_points());
        total_tests++;
        if (!float_approximates(trx_score, 0)) {
            total_errors++;
            wrn("Self score test failed: self score should be 0\n");
        }

        // Test randomiser
        const int N = 10;
        const int rand_trials = 50000000;
        const int expect_avg = rand_trials / N;
        const float rand_dev_tolerance = 0.05f * expect_avg; // 5% deviation

        int rand_count[N] = {0};
        for (size_t i = 0; i < rand_trials; i++) {
            const size_t dice = random::get_dice(N);
            total_tests++;
            if (dice >= N) {
                total_errors++;
                err("Failed to produce correct dice: random::get_dice() "
                    "produced %lu for [0-%lu)",
                    dice, N);
                break;
            }
            rand_count[dice]++;
        }

        for (size_t i = 0; i < N; i++) {
            const float rand_dev = static_cast<float>(
                                       abs(rand_count[i] - expect_avg) / (expect_avg));
            total_tests++;
            if (rand_dev > rand_dev_tolerance) {
                total_errors++;
                err("Too much random deviation: %.3f%% (expecting %d)\n",
                    rand_dev, expect_avg);
            }
        }

        // Test parallel randomiser
#ifndef _NO_OMP
        std::vector<uint32_t> para_rand_seeds = get_para_rand_seeds();
        const int n_threads = para_rand_seeds.size();
        const int para_rand_n = 8096;
        const int64_t dice_lim = 13377331;

        std::vector<uint32_t> rands1(para_rand_n);
        #pragma omp parallel for
        for (size_t i = 0; i < para_rand_n; i++)
            rands1.at(i) = random::get_dice(dice_lim);

        get_para_rand_seeds() = para_rand_seeds;
        std::vector<uint32_t> rands2(para_rand_n);
        #pragma omp parallel for
        for (size_t i = 0; i < para_rand_n; i++)
            rands2.at(i) = random::get_dice(dice_lim);

        for (size_t i = 0; i < para_rand_n; i++) {
            total_tests++;
            if (rands1.at(i) != rands2.at(i)) {
                total_errors++;
                err("Parallel randomiser failed: %d vs %d\n",
                    rands1.at(i), rands2.at(i));
            }
        }
#endif
    }

    msg("%lu/%lu integration tests passed.\n",
        (total_tests - total_errors), total_tests);
    return total_errors;
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