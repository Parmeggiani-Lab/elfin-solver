#ifndef VECTOR_UTILS_H_
#define VECTOR_UTILS_H_

#include <vector>

#include "debug_utils.h"
#include "random_utils.h"

namespace elfin {

template <typename T>
class Vector : public std::vector<T> {
private:
    /* types */
    typedef typename std::vector<T> ContainerType;
    typedef typename std::vector<T>::iterator ItrType;
    typedef typename std::vector<T>::const_iterator CItrType;

    /* data */

public:
    /* ctors */
    using ContainerType::ContainerType;

    /* dtors */

    /* accessors */
    CItrType find(const T & item) const {
        return const_cast<Vector<T> *>(this)->find(item);
    }

    bool contains(const T & item) const {
        return find(item) == this->end();
    }

    const T & rand_item() const {
        return const_cast<Vector<T> *>(this)->rand_item();
    }

    /* modifiers */
    ItrType find(const T & item) {
        return std::find(this->begin(), this->end(), item);
    }

    void lift_erase(ItrType & itr) {
        if (itr != this->end()) {
            *itr = std::move(this->back());
            this->pop_back();
        }
    }

    void lift_erase(const T & item) {
        ItrType itr = find(item);
        lift_erase(itr);
    }

    void lift_erase_all(
        const T & item,
        bool (*item_comparator)(const T &, const T &) = T::operator=) {
        for (size_t i = 0; i < this->size(); ++i) {
            if (item_comparator(this->at(i), item)) {
                this->at(i) = std::move(this->back());
                this->pop_back();
                i--; // need to check same index again
            }
        }
    }

    T & pick_random() {
        return random::pick(*this);
    }

    T pop_random() {
        return random::pop(*this);
    }

    /* printers */

};  /* class Vector*/

}  /* elfin */

#endif  /* end of include guard: VECTOR_UTILS_H_ */