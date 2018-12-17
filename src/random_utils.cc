#include "random_utils.h"

namespace elfin {

namespace random {

void init_seeds(uint32_t global_seed) {
    // Seending the RNG needs to be done per-thread because std::rand()
    // is not requied to be thread-safe
    #pragma omp parallel
    {
        #pragma omp single
        {
            RAND_SEEDS.resize(omp_get_num_threads(), 0);
        }

        const uint32_t seed =
            global_seed == 0 ?
            global_seed : get_timestamp_us();
        const int tid = omp_get_thread_num();

        RAND_SEEDS.at(tid) = seed ^ tid;
    }
}

void test(size_t& errors, size_t& tests) {
    /* Set up OMP */
    omp_set_dynamic(0); // Explicitly disable dynamic thread teams
    omp_set_num_threads(8); // Use exactly 8 threads

    #pragma omp parallel
    {
        #pragma omp single
        {
            msg("Testing random_utils (forcing %lu threads)\n",
                omp_get_num_threads());
        }
    }

    /* Test for randomizer consistency */
    std::vector<uint32_t> const original_seeds = RAND_SEEDS;

    size_t const N = 8096;
    size_t const dice_max = 13377331;

    std::vector<size_t> rand_vals1(N, 0);
    std::vector<size_t> rand_vals2(N, 0);

    #pragma omp parallel for
    for (size_t i = 0; i < N; ++i) {
        rand_vals1.at(i) = get_dice(dice_max);
    }
    std::vector<uint32_t> const seeds1 = RAND_SEEDS;

    RAND_SEEDS = original_seeds; // Restore to original seeds
    #pragma omp parallel for
    for (size_t i = 0; i < N; ++i) {
        rand_vals2.at(i) = get_dice(dice_max);
    }
    std::vector<uint32_t> const seeds2 = RAND_SEEDS;

    /*
     * Check that seeds are identical after the same number of calls to
     * get_dice().
     */
    tests++;
    for (size_t i = 0; i < RAND_SEEDS.size(); ++i) {
        if (seeds1.at(i) != seeds2.at(i)) {
            errors++;
            err("seeds1[%lu] = %lu, different from seeds2[%lu] = %lu\n",
                i, seeds1.at(i), i, seeds2.at(i));
            break;
        }
    }

    /*
     * Check that all random values originated from the same seeds are
     * identical.
     */
    tests++;
    for (size_t i = 0; i < N; ++i) {
        if (rand_vals1.at(i) != rand_vals2.at(i)) {
            errors++;
            err("Parallel randomiser failed: %lu vs %lu\n",
                rand_vals1.at(i), rand_vals2.at(i));
            break;
        }
    }
}

}  /* random */

}  /* elfin */