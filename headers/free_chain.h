#ifndef FREE_CHAIN_H
#define FREE_CHAIN_H

#include <functional>
#include <sstream>
#include <vector>

#include "terminus_type.h"
#include "vector_utils.h"
#include "proto_link.h"

namespace elfin {

class Node;

struct FreeChain {
    /* data */
    Node* node;
    TerminusType term;
    size_t chain_id;

    /* ctors */
    FreeChain() : FreeChain(nullptr, TerminusType::NONE, 0) {}
    FreeChain(
        Node* _node,
        TerminusType const _term,
        size_t const _chain_id);

    /* accessors */
    bool operator==(FreeChain const& other) const;
    bool operator!=(FreeChain const& other) const { return not this->operator==(other); }
    ProtoLink const& random_proto_link() const;

    /* printers */
    std::string to_string() const;
};

typedef Vector<FreeChain> FreeChainList;

}  /* elfin */

namespace std {

template <> struct hash<elfin::FreeChain> {
    size_t operator()(elfin::FreeChain const& x) const {
        return hash<void*>()((void*) x.node) ^
               hash<size_t>()(static_cast<int>(x.term)) ^
               hash<size_t>()(x.chain_id);
    }
};

} /* std */

namespace elfin {

// struct FreeChainVM : public VectorMap<FreeChain> {
//     std::string to_string() const;
// };

}  /* elfin */

#endif  /* end of include guard: FREE_CHAIN_H */