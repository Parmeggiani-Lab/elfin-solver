#ifndef LINK_H_
#define LINK_H_

#include <vector>

#include "free_chain.h"

namespace elfin {

class Link {
private:
    /* data */
    FreeChain src_chain, dst_chain;

    /* ctors */
    Link()
    { die("Not supposed to be called: %s\n", __PRETTY_FUNCTION__); }
public:
    /* ctors */
    Link(
        const FreeChain & src,
        const FreeChain & dst) : src_chain(src), dst_chain(dst) {}

    /* dtors */
    virtual ~Link() {}

    /* accessors */
    size_t hash() const {
        return std::hash<FreeChain>()(src_chain) ^
               std::hash<FreeChain>()(dst_chain);
    }
    /* getters */
    bool operator==(const Link & other) const;
    bool operator!=(const Link & other) const { return not this->operator==(other); }

    /* modifiers */

    /* printers */

};  /* class Link*/

}  /* elfin */

namespace std {

template <> struct hash<elfin::Link> {
    size_t operator()(const elfin::Link & x) const {
        return x.hash();
    }
};

}  /* std */

namespace elfin {

typedef VectorMap<Link> LinkVM;

}  /* elfin */

#endif  /* end of include guard: LINK_H_ */