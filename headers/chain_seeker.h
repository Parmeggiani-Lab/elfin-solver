#ifndef CHAIN_SEEKER_H_
#define CHAIN_SEEKER_H_

#include <functional>
#include <sstream>
#include <vector>

#include "terminus_type.h"
#include "vector_map.h"

namespace elfin {

class Node;

struct ChainSeeker {
    /* data */
    Node * node;
    size_t chain_id;

    /* ctors */
    ChainSeeker(Node * _node, const size_t _chain_id);

    /* getters */
    bool operator==(const ChainSeeker & other) const;
    bool operator!=(const ChainSeeker & other) const { return not (*this == other); }

    /* printers */
    std::string to_string() const;
};

typedef std::vector<ChainSeeker> ChainSeekerList;

}  /* elfin */

namespace std {
template <> struct hash<elfin::ChainSeeker> {
    size_t operator()(const elfin::ChainSeeker & x) const {
        return hash<void*>()((void*) x.node) ^ x.chain_id;
    }
};
} /* std */

#endif  /* end of include guard: CHAIN_SEEKER_H_ */