#ifndef RANDOM_UTILS_H_
#define RANDOM_UTILS_H_

#include <vector>
#include <random>

#include "debug_utils.h"

namespace elfin {

struct TestStat;

namespace random {

static inline float get_dice_0to1(uint32_t& seed) {
    return (float) rand_r(&seed) / RAND_MAX;
}

static inline size_t get_dice(size_t const ceiling, uint32_t& seed) {
    if (ceiling == 0) return 0;
    return std::round(get_dice_0to1(seed) * (ceiling - 1));
}


/* Container SFINAE helpers */
template<typename T>
struct has_at_method
{
private:
    typedef std::true_type yes;
    typedef std::false_type no;

    template<typename U> static auto test(int) -> decltype(std::declval<U>().at() == 1, yes());
    template<typename> static no test(...);

public:

    static constexpr bool value = std::is_same<decltype(test<T>(0)), yes>::value;
};


/* Generic Random Utility Functions */
template < class Container,
           std::enable_if < has_at_method<Container>::value and
                            !std::is_const<Container>::value > * = nullptr >
static inline
typename Container::value_type
pop(Container& v, uint32_t& seed) {
    DEBUG_NOMSG(v.empty());

    size_t const idx = get_dice(v.size(), seed);
    typename Container::value_type ret = v.at(idx);

    // https://stackoverflow.com/questions/9218724/get-random-element-and-remove-it
    std::swap(v[idx], v.back());
    v.pop_back();

    return ret;
}

template < typename Container,
           typename std::enable_if < has_at_method<Container>::value and
                                     !std::is_const<Container>::value,
                                     Container >::type* = nullptr >
static inline
typename Container::value_type &
pick(Container & v, uint32_t& seed) {
    DEBUG_NOMSG(v.empty());
    return v.at(get_dice(v.size(), seed));
}

template < typename Container,
           typename std::enable_if < !has_at_method<Container>::value and
                                     !std::is_const<Container>::value,
                                     Container >::type* = nullptr >
static inline
typename Container::value_type &
pick(Container & v, uint32_t& seed) {
    // O(n) complexity!
    DEBUG_NOMSG(v.empty());
    auto itr = begin(v);
    advance(itr, get_dice(v.size(), seed));
    return *itr;
}

// Const version of pick
template<class Container>
static inline
typename Container::value_type const &
pick(Container const& v, uint32_t& seed) {
    return pick(const_cast<Container&>(v), seed);
}

TestStat test();

}  /* random */

}  /* elfin */

#endif  /* end of include guard: RANDOM_UTILS_H_ */