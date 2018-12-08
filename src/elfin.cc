/* Copyright 2018 Joy Yeh <joyyeh@gmail.com> */

#include "elfin.h"

#include <string>
#include <sstream>
#include <csignal>
#include <iostream>
#include <fstream>

#include "input_manager.h"
#include "spec.h"
#include "database.h"
#include "kabsch.h"
#include "jutil.h"
#include "random_utils.h"
#include "parallel_utils.h"
#include "output_manager.h"

#ifndef _NO_OMP
#include <omp.h>
#endif

namespace elfin {

std::vector<ElfinRunner *> ElfinRunner::instances_;
bool interrupt_caught = false;

void ElfinRunner::interrupt_handler(const int signal) {
    if (interrupt_caught) {
        raw("\n\n");
        die("Caught interrupt signal (second)\n");
    }
    else {
        interrupt_caught = true;

        raw("\n\n");
        wrn("Caught interrupt signal (first)\n");

        // Save latest results
        for (auto er : instances_) {
            er->crash_dump();
            delete er;
        }

        exit(signal);
    }
}

void ElfinRunner::crash_dump() const {
    if (es_started_) {
        wrn("Dumping latest results...\n");
        OutputManager::write_output(solver_, "crash_dump");
        delete solver_;
    } else {
        wrn("GA did not get to start\n");
    }
}

int ElfinRunner::run_unit_tests() const {
    msg("Running unit tests...\n");
    int fail_count = 0;
    fail_count += _test_kabsch();
    return fail_count;
}

int ElfinRunner::run_meta_tests() const {
    msg("Running meta tests...\n");
    int fail_count = 0;

    for (auto & itr : SPEC.work_area_map()) {
        const WorkArea & wa = itr.second;
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
        if (!float_approximates(trx_score, 0)) {
            fail_count++;
            wrn("Self score test failed: self score should be 0\n");
        }

        // Test randomiser
        const int N = 10;
        const int rand_trials = 50000000;
        const int expect_avg = rand_trials / N;
        const float rand_dev_tolerance = 0.05f * expect_avg;  // 5% deviation

        int rand_count[N] = {0};
        for (size_t i = 0; i < rand_trials; i++) {
            const size_t dice = get_dice(N);
            if (dice >= N) {
                fail_count++;
                err("Failed to produce correct dice: get_dice() "
                    "produced %d for [0-%d)",
                    dice, N);
                break;
            }
            rand_count[dice]++;
        }

        for (size_t i = 0; i < N; i++) {
            const float rand_dev = static_cast<float>(
                                       abs(rand_count[i] - expect_avg) / (expect_avg));
            if (rand_dev > rand_dev_tolerance) {
                fail_count++;
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
            rands1.at(i) = get_dice(dice_lim);

        get_para_rand_seeds() = para_rand_seeds;
        std::vector<uint32_t> rands2(para_rand_n);
        #pragma omp parallel for
        for (size_t i = 0; i < para_rand_n; i++)
            rands2.at(i) = get_dice(dice_lim);

        for (size_t i = 0; i < para_rand_n; i++) {
            if (rands1.at(i) != rands2.at(i)) {
                fail_count++;
                err("Parallel randomiser failed: %d vs %d\n",
                    rands1.at(i), rands2.at(i));
            }
        }
#endif

        if (fail_count > 0)
            break;
    }

    return fail_count;
}

ElfinRunner::ElfinRunner(const int argc, const char ** argv) {
    instances_.push_back(this);

    std::signal(SIGINT, interrupt_handler);

    set_log_level(LOG_WARN);

    InputManager::setup(argc, argv);
}

void ElfinRunner::run() {
    if (OPTIONS.run_unit_tests) {
        int fail_count = 0;
        fail_count += run_unit_tests();
        fail_count += run_meta_tests();

        if (fail_count > 0) {
            die("Some unit tests failed\n");
        } else {
            msg("Passed!\n");
        }
    } else {
        solver_ = new EvolutionSolver();
        es_started_ = true;
        solver_->run();
        OutputManager::write_output(solver_);

        delete solver_;
    }
}

}  // namespace elfin

int main(const int argc, const char ** argv) {
    elfin::ElfinRunner(argc, argv).run();
    return 0;
}
