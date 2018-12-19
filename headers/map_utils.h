#ifndef MAP_UTILS_H_
#define MAP_UTILS_H_

#include <unordered_map>
#include <memory>

namespace elfin {

template<class T, typename KeyType = std::string>
using UPMap = std::unordered_map <
              KeyType,
              std::unique_ptr<T >>;

template<class T, typename KeyType = std::string>
using SPMap = std::unordered_map <
              KeyType,
              std::shared_ptr<T >>;

}  /* elfin */

#endif  /* end of include guard: MAP_UTILS_H_ */