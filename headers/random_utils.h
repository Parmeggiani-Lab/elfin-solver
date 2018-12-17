#ifndef RANDOM_UTILS_H_
#define RANDOM_UTILS_H_

#include <vector>

#include "parallel_utils.h"
#include "debug_utils.h"

namespace elfin {

namespace random {

namespace {

std::vector<uint32_t> RAND_SEEDS;

}  /* (anonymous) */

void init_seeds(uint32_t global_seed);

static inline float get_dice_0to1() {
    uint32_t& thread_seed = RAND_SEEDS.at(omp_get_thread_num());
    return (float) rand_r(&thread_seed) / RAND_MAX;
}

static inline size_t get_dice(size_t ceiling) {
    return (size_t) std::round(get_dice_0to1() * (ceiling ? ceiling - 1 : 0));
}

// https://stackoverflow.com/questions/9218724/get-random-element-and-remove-it
template <typename T>
static inline void remove_at(std::vector<T> & v, typename std::vector<T>::size_type n) {
    DEBUG(v.empty());
    std::swap(v[n], v.back());
    v.pop_back();
}

template <typename T>
static inline T pop(std::vector<T> & v) {
    DEBUG(v.empty());
    size_t const idx = get_dice(v.size());
    T ret = v.at(idx);
    remove_at(v, idx);
    return ret;
}

template <typename T>
static inline T const& pick(const std::vector<T> & v) {
    DEBUG(v.empty());
    size_t const idx = get_dice(v.size());
    return v.at(idx);
}

template <typename T>
static inline T& pick(std::vector<T> & v) {
    DEBUG(v.empty());
    size_t const idx = get_dice(v.size());
    return v.at(idx);
}

void test(size_t& errors, size_t& tests);

}

}  /* elfin */

#endif  /* end of include guard: RANDOM_UTILS_H_ */