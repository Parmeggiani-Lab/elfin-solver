#ifndef NODE_H_
#define NODE_H_

#include <memory>
#include <list>

#include "proto_module.h"
#include "geometry.h"
#include "link.h"

namespace elfin {

/* Fwd Decl */
class Node;
typedef std::unique_ptr<Node> NodeSP;
typedef Node const* NodeKey;

class Node : public Printable {
protected:
    //
    // Note: Use of vector as container for neighbors (LinkList)
    //
    // I chose to use vector because as far as we are concerned now, modules
    // have very few termini e.g. max 4 at the time of writing. Although it's
    // O(n) to access the wanted FreeTerm, using more complex structure to
    // support O(1) operations is probably not worth the memory/dev time due
    // to how tiny the vectors are.
    //
    // If in the future the number of termini per module does increase
    // significantly, it might be advisable to switch to VectorMap or
    // something better.
    //

    /* data */
    std::list<Link> links_;

public:
    /* data */
    Transform tx_;
    ProtoModule const* const prototype_;

    /* ctors */
    Node(ProtoModule const* prototype, Transform const& tx) :
        prototype_(prototype), tx_(tx) {}
    Node(ProtoModule const* const prototype) : Node(prototype, Transform()) {}
    NodeSP clone() const { return std::make_unique<Node>(*this); }

    /* accessors */
    std::list<Link> const& links() const { return links_; }
    Link const* find_link_to(NodeKey dst_node) const;

    /* modifiers */
    void add_link(
        FreeTerm const& src,
        ProtoLink const* proto_link,
        FreeTerm const& dst) {
        links_.emplace_back(src, proto_link, dst);
    }
    void add_link(Link const& link) {
        links_.emplace_back(link);
    }
    void update_link_ptrs(NodeKeyMap const& nam);
    void remove_link(FreeTerm const& fc);

    /* printers */
    virtual void print_to(std::ostream& os) const;
};

}  /* elfin */

#endif  /* end of include guard: NODE_H_ */