#ifndef PRIV_IMPL_H_
#define PRIV_IMPL_H_

#include <memory>

namespace elfin {

template<class T>
struct PImplBase {
    /* types */
    typedef PImplBase<T> pimpl_t;
    // Data
    T& _;  // Owner back reference.

    /* ctors */
    PImplBase(T& owner) : _(owner) { }

    /* dtors */
    virtual ~PImplBase() {}
};

template<typename pimpl_t, class T>
inline std::unique_ptr<pimpl_t> new_pimpl(T& owner) {
    return std::make_unique<pimpl_t>(owner);
}

}  /* elfin */

#endif  /* end of include guard: PRIV_IMPL_H_ */