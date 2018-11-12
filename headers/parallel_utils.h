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

inline long TIMING_END(const char * sectionName, const double startTime) {
	const long diff = (long) ((get_timestamp_us() - startTime) / 1e3);
	msg("Section (%s) time: %.dms\n", sectionName, diff);
	return diff;
}

#else //ifdef _DO_TIMING

#define TIMING_START(varName)
#define TIMING_END(sectionName, varName)

#endif //ifdef _DO_TIMING


namespace elfin
{

void set_thread_seeds(uint global_seed);

std::vector<uint> & get_para_rand_seeds();

} // namespace elfin

#endif /* include guard */
