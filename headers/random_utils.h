#ifndef RANDOM_UTILS_H_
#define RANDOM_UTILS_H_

#include <vector>

#include "parallel_utils.h"

namespace elfin {

inline size_t get_dice(size_t ceiling)
{
    uint & thread_seed = get_para_rand_seeds().at(omp_get_thread_num());
    return (size_t) std::round(
               (float) (ceiling ? ceiling - 1 : 0) *
               rand_r(&thread_seed) / RAND_MAX);
}

// https://stackoverflow.com/questions/9218724/get-random-element-and-remove-it
template <typename T>
inline void remove_at(std::vector<T> & v, typename std::vector<T>::size_type n) {
    std::swap(v[n], v.back());
    v.pop_back();
}

template <typename T>
inline T pop_random(std::vector<T> & v) {
    const size_t idx = get_dice(v.size());
    T ret = v.at(idx);
    remove_at(v, idx);
    return ret;
}

template <typename T>
inline T pick_random(const std::vector<T> & v) {
    const size_t idx = get_dice(v.size());
    return v.at(idx);
}

}  /* elfin */

#endif  /* end of include guard: RANDOM_UTILS_H_ */