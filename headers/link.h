#ifndef LINK_H_
#define LINK_H_

#include "free_chain.h"
#include "vector_utils.h"
#include "proto_link.h"

namespace elfin {

class Node;
typedef std::unordered_map<const Node *, Node *> NodeAddrMap;

class Link {
private:
    /* data */
    FreeChain src_chain_, dst_chain_;
    const ProtoLink * prototype_;
    
public:
    /* ctors */
    Link() = delete;
    Link(
        const FreeChain & src_chain,
        const ProtoLink * prototype,
        const FreeChain & dst_chain);
    Link reversed() const {
        return Link(dst_chain_, prototype_->reverse(), src_chain_);
    }

    /* dtors */
    virtual ~Link() {}

    /* accessors */
    const FreeChain & src() const { return src_chain_; }
    const FreeChain & dst() const { return dst_chain_; }
    const ProtoLink * prototype() const { return prototype_; }
    size_t hash() const;
    bool operator==(const Link & other) const;

    /* modifiers */
    void update_node_ptrs(const NodeAddrMap & nam);
    static void sever(const Link link);

    /* printers */
    std::string to_string() const;
};  /* class Link*/

typedef Vector<Link> LinkList;

}  /* elfin */

namespace std {

template <> struct hash<elfin::Link> {
    size_t operator()(const elfin::Link & x) const {
        return std::hash<elfin::FreeChain>()(x.src()) ^
               std::hash<elfin::FreeChain>()(x.dst());
    }
};

}  /* std */

// namespace elfin {

// typedef VectorMap<Link> LinkVM;

// }  /* elfin */

#endif  /* end of include guard: LINK_H_ */