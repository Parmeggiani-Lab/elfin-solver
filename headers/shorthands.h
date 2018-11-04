#ifndef SHORTHANDS_H_
#define SHORTHANDS_H_

#include <vector>
#include <unordered_map>

namespace elfin {

typedef std::unordered_map<std::string, long> NameIdMap;
typedef std::unordered_map<long, std::string> IdNameMap;

extern const NameIdMap & NAME_ID_MAP; // defined in elfin.cc
extern const IdNameMap & ID_NAME_MAP; // defined in elfin.cc

typedef uint32_t uint;
typedef uint64_t ulong;

}  /* elfin */

#endif  /* end of include guard: SHORTHANDS_H_ */