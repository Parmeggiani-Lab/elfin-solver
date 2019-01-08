#ifndef FREE_TERM_H
#define FREE_TERM_H

#include <functional>
#include <sstream>
#include <vector>

#include "term_type.h"
#include "proto_link.h"

namespace elfin {

class Node;
typedef Node const* NodeKey;

struct FreeTerm : public Printable {
    /* types */
    struct Bridge {
        ProtoLink const* const pt_link1, * const pt_link2;
        Bridge(ProtoLink const* _pt_link1, ProtoLink const*_pt_link2) :
            pt_link1(_pt_link1), pt_link2(_pt_link2) {}
    };

    typedef std::vector<Bridge> BridgeList;

    /* data */
    NodeKey node;
    TermType term;
    size_t chain_id;
    bool should_restore = true;

    /* ctors */
    FreeTerm(NodeKey const _node,
             TermType const _term,
             size_t const _chain_id);
    FreeTerm() : FreeTerm(nullptr, TermType::NONE, 0) {}

    /* dtors */
    virtual ~FreeTerm() {}

    /* accessors */
    bool operator==(FreeTerm const& other) const;
    bool operator!=(FreeTerm const& other) const { return not this->operator==(other); }
    ProtoLink const& random_proto_link() const;
    BridgeList find_bridges(FreeTerm const& dst) const;
    ProtoLink const* find_link_to(FreeTerm const& dst) const;

    /* printers */
    virtual void print_to(std::ostream& os) const;
};

typedef std::vector<FreeTerm> FreeTerms;

}  /* elfin */

namespace std {

template <> struct hash<elfin::FreeTerm> {
    size_t operator()(elfin::FreeTerm const& x) const {
        return hash<void*>()((void*) x.node) ^
               hash<size_t>()(static_cast<int>(x.term)) ^
               hash<size_t>()(x.chain_id);
    }
};

} /* std */

#endif  /* end of include guard: FREE_TERM_H */