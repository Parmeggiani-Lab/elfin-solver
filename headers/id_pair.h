#ifndef ID_PAIR_H_
#define ID_PAIR_H_

#include <vector>
#include <cstddef>

namespace elfin {

struct IdPair {
    size_t x, y;
    IdPair() : x(0), y(0) {};
    IdPair(size_t _x, size_t _y) : x(_x), y(_y) {};
};

typedef std::vector<IdPair> IdPairs;

}  /* elfin */

#endif  /* end of include guard: ID_PAIR_H_ */