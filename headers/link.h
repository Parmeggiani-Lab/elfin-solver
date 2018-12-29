#ifndef LINK_H_
#define LINK_H_

#include "free_chain.h"
#include "vector_utils.h"
#include "proto_link.h"

namespace elfin {

/* Fwd Decl */
class Node;
typedef std::unique_ptr<Node> NodeSP;
typedef std::unordered_map<Node const*, Node*> NodeAddrMap;
class PathGenerator;

class Link : public Printable {
private:
    /* data */
    FreeChain src_chain_, dst_chain_;
    ProtoLink const* prototype_;

public:
    /* ctors */
    Link() = delete;
    Link(
        FreeChain const& src_chain,
        ProtoLink const* prototype,
        FreeChain const& dst_chain);
    Link reversed() const {
        return Link(dst_chain_, prototype_->reverse(), src_chain_);
    }

    /* accessors */
    FreeChain const& src() const { return src_chain_; }
    FreeChain const& dst() const { return dst_chain_; }
    ProtoLink const* prototype() const { return prototype_; }
    size_t hash() const;
    bool operator==(Link const& other) const;
    bool operator!=(Link const& other) const { return not this->operator==(other); }
    PathGenerator gen_path() const;

    /* modifiers */
    void update_node_ptrs(NodeAddrMap const& nam);

    /* printers */
    virtual void print_to(std::ostream& os) const;
};  /* class Link */

typedef Vector<Link> LinkList;

}  /* elfin */

namespace std {

template <> struct hash<elfin::Link> {
    size_t operator()(const elfin::Link& x) const {
        return std::hash<elfin::FreeChain>()(x.src()) ^
               std::hash<elfin::FreeChain>()(x.dst());
    }
};

}  /* std */

#endif  /* end of include guard: LINK_H_ */