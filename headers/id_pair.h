#ifndef ID_PAIR_H_
#define ID_PAIR_H_

#include <vector>

namespace elfin {

struct IdPair {
    long x, y;
    IdPair() : x(0), y(0) {};
    IdPair(long _x, long _y) : x(_x), y(_y) {};
};

typedef std::vector<IdPair> IdPairs;
typedef std::vector<std::tuple<IdPair, IdPairs>> CrossingVector;

}  /* elfin */

#endif  /* end of include guard: ID_PAIR_H_ */