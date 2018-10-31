#include <stdlib.h>

#include "parallel_utils.h"

namespace elfin
{

std::vector<uint> para_rand_seeds;

void set_thread_seeds(uint global_seed)
{
	// Seending the RNG needs to be done per-thread because std::rand()
	// is not requied to be thread-safe
	#pragma omp parallel
	{
		#pragma omp single
		{
			para_rand_seeds.resize(omp_get_num_threads());
		}

		para_rand_seeds.at(omp_get_thread_num()) = global_seed == 0 ?
		        get_timestamp_us() : (global_seed + omp_get_thread_num());
	}
}

std::vector<uint> & get_para_rand_seeds()
{
	return para_rand_seeds;
}

} // namespace elfin