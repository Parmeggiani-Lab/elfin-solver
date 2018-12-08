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
    typedef typename std::vector<T>::iterator ItrType;
    typedef typename std::vector<T>::const_iterator CItrType;

    /* data */

public:
    /* ctors */
    // Vector() {}

    /* dtors */
    // virtual ~Vector() {}

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
        DEBUG(itr == this->end());
        DEBUG(this->empty());

        *itr = std::move(this->back());
        this->pop_back();
    }

    void lift_erase(const T & item) {
        ItrType itr = find(item);
        lift_erase(itr);
    }

    T & rand_item() {
        return pick_random(*this);
    }

    /* printers */

};  /* class Vector*/

}  /* elfin */

#endif  /* end of include guard: VECTOR_UTILS_H_ */