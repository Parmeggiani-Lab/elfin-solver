#ifndef POINTER_UTILS_H_
#define POINTER_UTILS_H_

#include <memory>

namespace elfin {

template <typename T>
bool is_uninitialized(std::weak_ptr<T> const& weak) {
    using wt = std::weak_ptr<T>;
    return !weak.owner_before(wt{}) && !wt{}.owner_before(weak);
}

}  /* elfin */

#endif  /* end of include guard: POINTER_UTILS_H_ */