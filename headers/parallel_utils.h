#ifndef PARALLEL_UTILS_H_
#define PARALLEL_UTILS_H_

#include <omp.h>

#include <jutil/jutil.h>

/* OMP Macros */
#define OMP_PAR_FOR _Pragma("omp parallel for simd schedule(runtime)")

#define TIMING_START(var_name) \
    double const var_name = get_timestamp_us();

inline long TIMING_END(char const* section_name, double const start_time) {
    long const diff = (long) ((get_timestamp_us() - start_time) / 1e3);
    msg("Section (%s) time: %ldms\n", section_name, diff);
    return diff;
}

namespace elfin {

namespace parallel {

void init();

}  /* parallel */

}  /* elfin */

#endif  /* end of include guard: PARALLEL_UTILS_H_ */