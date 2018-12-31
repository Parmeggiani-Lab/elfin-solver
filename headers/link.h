#ifndef LINK_H_
#define LINK_H_

#include "free_chain.h"
#include "proto_link.h"

namespace elfin {

/* Fwd Decl */
class Node;
typedef std::unique_ptr<Node> NodeSP;
typedef std::unordered_map<Node const*, Node*> NodeKeyMap;
class PathGenerator;

class Link : public Printable {
protected:
    /* data */
    FreeChain src_;
    FreeChain dst_;

public:
    /* data */
    ProtoLink const* const prototype_;

    /* ctors */
    Link(FreeChain const& src,
         ProtoLink const* prot,
         FreeChain const& dst);
    Link(Link const& other);
    Link reversed() const {
        return Link(dst_, prototype_->reverse(), src_);
    }

    /* dtors */
    virtual ~Link() {}

    /* accessors */
    FreeChain const& src() const { return src_; }
    FreeChain const& dst() const { return dst_; }
    // bool operator==(Link const& other) const;
    // bool operator!=(Link const& other) const { return not this->operator==(other); }
    PathGenerator gen_path() const;

    /* modifiers */
    void update_node_ptrs(NodeKeyMap const& nam);

    /* printers */
    virtual void print_to(std::ostream& os) const;
};  /* class Link */

}  /* elfin */

// namespace std {

// template <> struct hash<elfin::Link> {
//     size_t operator()(const elfin::Link& x) const {
//         return std::hash<elfin::FreeChain>()(x.src()) ^
//                std::hash<elfin::FreeChain>()(x.dst());
//     }
// };

// }  /* std */

#endif  /* end of include guard: LINK_H_ */