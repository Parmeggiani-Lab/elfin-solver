#include "random_utils.h"

#include "input_manager.h"

namespace elfin {

namespace random {

namespace {

std::vector<std::mt19937> TWISTERS;

}  /* (anonymous) */

void init() {
    TWISTERS.clear();

    info("Using master seed: 0x%p\n", OPTIONS.seed);

    #pragma omp parallel
    {
        #pragma omp single
        {
            size_t const num_threads = omp_get_num_threads();
            dbg("Creating %zu Mersenne Twisters\n", num_threads);

            uint32_t global_seed = OPTIONS.seed;
            if (global_seed == 0) {
                global_seed = get_timestamp_us();
            }

            for (size_t tid = 0; tid < num_threads; ++tid) {
                TWISTERS.emplace_back((uint32_t) global_seed ^ tid);
            }
        }
    }
}

float get_dice_0to1() {
    DEBUG(omp_get_thread_num() >= TWISTERS.size(),
          string_format("Thread #%d; TWISTERS size: %zu\n",
                        omp_get_thread_num(), TWISTERS.size()));
    std::mt19937& mt = TWISTERS.at(omp_get_thread_num());
    return (float) mt() / mt.max();
}

}  /* random */

}  /* elfin */