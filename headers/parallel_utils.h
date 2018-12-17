#ifndef PARALLEL_UTILS_H_
#define PARALLEL_UTILS_H_

#include <vector>
#include <cmath>

#include <omp.h>

#include "jutil.h"

/* OMP Macros */
#ifdef NO_OMP

#define OMP_PAR_FOR
#define MAP_DATA()

#else  /* ifdef NO_OMP */

#ifdef TARGET_GPU

#error "Not implemented"
// #define OMP_PAR_FOR _Pragma("omp target teams distribute parallel for simd schedule(static,1)")
// #define MAP_DATA() _Pragma("omp target data map(buff_pop_data_[0:pop_size_], curr_pop_data_[0:pop_size_])")

#else  /* ifdef TARGET_GPU */

#define OMP_PAR_FOR _Pragma("omp parallel for simd schedule(runtime)")
#define MAP_DATA() {}

#endif  /* ifdef TARGET_GPU */

#endif  /* ifdef NO_OMP */

/* Timing Macros */
#ifdef DO_TIMING

#define TIMING_START(var_name) \
    double const var_name = get_timestamp_us();

inline long TIMING_END(char const* section_name, double const start_time) {
    long const diff = (long) ((get_timestamp_us() - start_time) / 1e3);
    msg("Section (%s) time: %ldms\n", section_name, diff);
    return diff;
}

#else //ifdef DO_TIMING

#define TIMING_START(var_name)
#define TIMING_END(section_name, var_name)

#endif //ifdef DO_TIMING

#endif  /* end of include guard: PARALLEL_UTILS_H_ */