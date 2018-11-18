#ifndef MAP_TYPES_H_
#define MAP_TYPES_H_

namespace elfin {

typedef std::unordered_map<std::string, size_t> StrIdMap;
typedef std::unordered_map<size_t, std::string> IdStrMap;

}  /* elfin */

#endif  /* end of include guard: MAP_TYPES_H_ */