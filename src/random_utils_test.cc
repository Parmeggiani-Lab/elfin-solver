#include "random_utils.h"

#include "omp.h"

#include "test_stat.h"
#include "jutil.h"

namespace elfin {

namespace random {

/* tests */
TestStat test() {
    TestStat ts;

    size_t const N = 37189;
    size_t const dice_max = 13377331;
    uint32_t const test_seed = 0xeeee;

    // Test single thread consistency.
    {
        ts.tests++;
        auto seed = test_seed;
        for (size_t i = 0; i < N; ++i) {
            auto seed2 = seed;
            auto const v1 = rand_r(&seed);
            auto const v2 = rand_r(&seed2);
            if (v1 != v2) {
                ts.errors++;
                JUtil.error("rand_r() inconsistent at #%zu: %zu vs %zu\n",
                            i, v1, v2);
                break;
            }
        }
    }

    // Test multi-threaded consistency.
    {
        // Set up OMP.
        omp_set_dynamic(0);                 // Explicitly disable dynamic thread teams.
        size_t const num_threads = 9;       // Use a weird number.
        omp_set_num_threads(num_threads);   // Use exactly N threads.

        auto seed = test_seed;
        std::vector<uint32_t> seeds(N, 0);
        for (size_t i = 0; i < N; ++i) {
            seeds[i] = rand_r(&seed);
        }

        std::vector<size_t> vals1(N, 0);
        #pragma omp parallel for
        for (size_t i = 0; i < N; ++i) {
            auto seed_copy = seeds.at(i);
            vals1[i] = get_dice(dice_max, seed_copy);
        }

        std::vector<size_t> vals2(N, 0);
        #pragma omp parallel for
        for (size_t i = 0; i < N; ++i) {
            auto seed_copy = seeds.at(i);
            vals2[i] = get_dice(dice_max, seed_copy);
        }

        // Check that all random values originated from the same seeds are
        // identical.
        ts.tests++;
        for (size_t i = 0; i < N; ++i) {
            if (vals1.at(i) != vals2.at(i)) {
                ts.errors++;
                JUtil.error("Parallel randomiser failed at #%zu: %zu vs %zu\n",
                            i, vals1.at(i), vals2.at(i));
                break;
            }
        }
    }

    return ts;
}

}  /* random */

}  /* elfin */