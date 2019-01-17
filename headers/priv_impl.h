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
    PImplBase(T& owner) : _(owner) {}

    /* dtors */
    virtual ~PImplBase() {}
};

template<typename pimpl_t, typename ... Args>
inline std::unique_ptr<pimpl_t> new_pimpl(Args&& ... args) {
    return std::make_unique<pimpl_t>(std::forward<Args>(args)...);
}

}  /* elfin */

#endif  /* end of include guard: PRIV_IMPL_H_ */