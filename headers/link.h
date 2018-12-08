#ifndef LINK_H_
#define LINK_H_

#include "free_chain.h"
#include "vector_utils.h"

namespace elfin {

class Node;
typedef std::unordered_map<const Node *, Node *> NodeAddrMap;

class Link {
private:
    /* data */
    FreeChain src_chain_, dst_chain_;

    /* ctors */
    Link()
    { die("Not supposed to be called: %s\n", __PRETTY_FUNCTION__); }
public:
    /* ctors */
    Link(
        const FreeChain & src,
        const FreeChain & dst) : src_chain_(src), dst_chain_(dst) {}

    /* dtors */
    virtual ~Link() {}

    /* accessors */
    const FreeChain & src() const { return src_chain_; }
    const FreeChain & dst() const { return dst_chain_; }
    size_t hash() const {
        return std::hash<FreeChain>()(src_chain_) ^
               std::hash<FreeChain>()(dst_chain_);
    }
    /* getters */
    bool operator==(const Link & other) const;
    bool operator!=(const Link & other) const { return not this->operator==(other); }

    /* modifiers */
    void update_node_ptrs(const NodeAddrMap & nam);

    /* printers */

};  /* class Link*/

typedef Vector<Link> LinkList;

}  /* elfin */

namespace std {

template <> struct hash<elfin::Link> {
    size_t operator()(const elfin::Link & x) const {
        return x.hash();
    }
};

}  /* std */

// namespace elfin {

// typedef VectorMap<Link> LinkVM;

// }  /* elfin */

#endif  /* end of include guard: LINK_H_ */