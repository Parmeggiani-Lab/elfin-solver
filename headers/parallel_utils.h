#ifndef PARALLELUTILS_H
#define PARALLELUTILS_H

#include <vector>
#include <cmath>

#ifndef _NO_OMP
#include <omp.h>
#endif

#include "jutil.h"

#ifdef _NO_OMP

inline int omp_get_thread_num() { return 0; }
inline int omp_get_num_threads() { return 1; }
inline int omp_get_num_devices() { return 0; }
inline int omp_set_default_device(int d) { return 0; }
inline int omp_get_max_threads() { return 1; }
inline int omp_get_initial_device() { return 0; }

#define OMP_PAR_FOR
#define MAP_DATA()

#else

#ifdef _TARGET_GPU
#define OMP_PAR_FOR _Pragma("omp target teams distribute parallel for simd schedule(static,1)")
#define MAP_DATA() _Pragma("omp target data map(buff_pop_data_[0:pop_size_], curr_pop_data_[0:pop_size_])")
#else
#define OMP_PAR_FOR _Pragma("omp parallel for simd schedule(runtime)")
#define MAP_DATA() {}
#endif

#endif

#ifdef _DO_TIMING
#define TIMING_START(varName) \
	const double varName = get_timestamp_us();

inline long TIMING_END(const char * section_name, const double start_time) {
	const long diff = (long) ((get_timestamp_us() - start_time) / 1e3);
	msg("Section (%s) time: %ldms\n", section_name, diff);
	return diff;
}

#else //ifdef _DO_TIMING

#define TIMING_START(var_name)
#define TIMING_END(section_name, var_name)

#endif //ifdef _DO_TIMING


namespace elfin {

void set_thread_seeds(uint32_t global_seed);

std::vector<uint32_t> & get_para_rand_seeds();

} // namespace elfin

#endif /* include guard */
