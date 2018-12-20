#include "random_utils.h"

#include "test_stat.h"


namespace elfin {

namespace random {

/* test data */

/* tests */
TestStat test() {
    TestStat ts;

    // Set up OMP.
    omp_set_dynamic(0);                 // Explicitly disable dynamic thread teams.
    size_t const num_threads = 9;       // Use a weird number.
    omp_set_num_threads(num_threads);   // Use exactly N threads.

    #pragma omp parallel
    {
        #pragma omp single
        {
            size_t const n = omp_get_num_threads();
            msg("Testing random_utils (forcing %lu threads)\n", n);

            NICE_PANIC(n != num_threads,
            string_format(
                "Faild to set number of threads! Set %lu, got %lu",
                num_threads, n));
        }
    }

    // Test single thread mt consistency.
    size_t const N = 3719;
    size_t const dice_max = 13377331;
    uint32_t const mt_seed = 0xeeee;

    std::mt19937 mt1(mt_seed);
    std::mt19937 mt2(mt_seed);

    ts.tests++;
    for (size_t i = 0; i < N; ++i) {
        size_t const v1 = mt1(), v2 = mt2();
        if (v1 != v2) {
            ts.errors++;
            err("MT19937 failed at #%lu: %lu vs %lu\n",
                i, v1, v2);
            break;
        }
    }

    std::vector<size_t> rand_vals1(N, 0);
    std::vector<size_t> rand_vals2(N, 0);

    init();
    #pragma omp parallel for
    for (size_t i = 0; i < N; ++i) {
        rand_vals1.at(i) = get_dice(dice_max);
    }

    init();
    #pragma omp parallel for
    for (size_t i = 0; i < N; ++i) {
        rand_vals2.at(i) = get_dice(dice_max);
    }

    // Check that all random values originated from the same seeds are
    // identical.
    ts.tests++;
    for (size_t i = 0; i < N; ++i) {
        if (rand_vals1.at(i) != rand_vals2.at(i)) {
            ts.errors++;
            err("Parallel randomiser failed at #%lu: %lu vs %lu\n",
                i, rand_vals1.at(i), rand_vals2.at(i));
            break;
        }
    }

    return ts;
}

}  /* random */

}  /* elfin */