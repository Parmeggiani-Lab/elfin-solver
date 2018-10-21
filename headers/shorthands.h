#ifndef SHORTHANDS_H_
#define SHORTHANDS_H_

#include <unordered_map>

namespace elfin {

typedef std::unordered_map<std::string, long> NameIdMap;
typedef std::unordered_map<long, std::string> IdNameMap;

typedef std::vector<long> IdRoulette;
typedef std::vector<long> Ids;

typedef uint32_t uint;
typedef uint64_t ulong;

}  /* elfin */

#endif  /* end of include guard: SHORTHANDS_H_ */