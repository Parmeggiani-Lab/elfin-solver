#include "random_utils.h"

#include "input_manager.h"

namespace elfin {

namespace random {

namespace {

std::vector<uint32_t> RAND_SEEDS;

}  /* (anonymous) */

void init(uint32_t global_seed) {
    /*
     * Create per-thread seed, because std::rand() is not required to be
     * thread-safe.
     */
    size_t num_threads = 1;
    #pragma omp parallel
    {
        #pragma omp single
        {
            num_threads = omp_get_num_threads();
            RAND_SEEDS.resize(num_threads, 0);
        }
    }

    if (global_seed == 0)
        global_seed = get_timestamp_us();

    for (size_t tid = 0; tid < num_threads; ++tid) {
        RAND_SEEDS.at(tid) = global_seed ^ tid;
    }
}

float get_dice_0to1() {
    DEBUG(omp_get_thread_num() >= RAND_SEEDS.size(),
          string_format("Thread #%d; RAND_SEEDS size: %lu\n",
                        omp_get_thread_num(), RAND_SEEDS.size()));
    uint32_t& thread_seed = RAND_SEEDS.at(omp_get_thread_num());
    return (float) rand_r(&thread_seed) / RAND_MAX;
}

void test(size_t& errors, size_t& tests) {
    /* Set up OMP */
    omp_set_dynamic(0); // Explicitly disable dynamic thread teams
    size_t const num_threads = 9; // Use a weird number
    omp_set_num_threads(num_threads); // Use exactly N threads

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

    /* Test multi-thread init() consistency */
    init(OPTIONS.rand_seed);
    std::vector<uint32_t> const init_seeds1 = RAND_SEEDS;
    init(OPTIONS.rand_seed);
    std::vector<uint32_t> const init_seeds2 = RAND_SEEDS;

    tests++;
    for (size_t i = 0; i < num_threads; ++i) {
        if (init_seeds1.at(i) != init_seeds2.at(i)) {
            errors++;
            err("random::init() is inconsistent!\n");
            break;
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