#ifndef FREE_CHAIN_H
#define FREE_CHAIN_H

#include <functional>
#include <sstream>
#include <vector>

#include "terminus_type.h"
#include "vector_map.h"

namespace elfin {

class Node;

struct FreeChain {
    /* data */
    Node * node;
    TerminusType term;
    size_t chain_id;

    /* ctors */
    FreeChain() : FreeChain(nullptr, TerminusType::NONE, 0) {}
    FreeChain(
        Node * _node,
        const TerminusType _term,
        const size_t _chain_id);

    /* accessors */
    bool operator==(const FreeChain & other) const;
    bool operator!=(const FreeChain & other) const { return not this->operator==(other); }

    /* printers */
    std::string to_string() const;
};

}  /* elfin */

namespace std {

template <> struct hash<elfin::FreeChain> {
    size_t operator()(const elfin::FreeChain & x) const {
        return hash<void*>()((void*) x.node) ^
               hash<size_t>()(x.term) ^
               hash<size_t>()(x.chain_id);
    }
};

} /* std */

namespace elfin {

struct FreeChainVM : public VectorMap<FreeChain> {
    std::string to_string() const;
};

}  /* elfin */

#endif  /* end of include guard: FREE_CHAIN_H */