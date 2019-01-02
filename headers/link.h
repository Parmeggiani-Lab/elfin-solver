#ifndef LINK_H_
#define LINK_H_

#include "free_term.h"
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
    FreeTerm src_;
    FreeTerm dst_;
    ProtoLink const* prototype_;

public:

    /* ctors */
    Link(FreeTerm const& src,
         ProtoLink const* prot,
         FreeTerm const& dst);
    Link reversed() const {
        return Link(dst_, prototype_->reverse(), src_);
    }

    /* dtors */
    virtual ~Link() {}

    /* accessors */
    FreeTerm const& src() const { return src_; }
    FreeTerm const& dst() const { return dst_; }
    ProtoLink const* prototype() const { return prototype_; }
    // bool operator==(Link const& other) const;
    // bool operator!=(Link const& other) const { return not this->operator==(other); }
    PathGenerator gen_path() const;

    /* modifiers */
    void update_node_ptrs(NodeKeyMap const& nam);

    /* printers */
    virtual void print_to(std::ostream& os) const;
};  /* class Link */

}  /* elfin */

#endif  /* end of include guard: LINK_H_ */