#ifndef CHAIN_SEEKER_H_
#define CHAIN_SEEKER_H_

#include <functional>
#include <sstream>

namespace elfin {

class Node;

struct ChainSeeker {
    /* data */
    const Node * node;
    size_t chain_id;

    /* ctors */
    ChainSeeker(const Node * _node, const size_t _chain_id) :
        node(_node), chain_id(_chain_id) {}

    /* getters */
    bool operator==(const ChainSeeker & other) const {
        return node == other.node && chain_id == other.chain_id;
    }

    /* printers */
    std::string to_string() const {
        std::ostringstream ss;
        ss << "ChainSeeker[Node: 0x" << std::hex << node << std::dec;
        ss << ", Chain Id: " << chain_id << "]\n";
        return ss.str();
    }
};

}  /* elfin */

namespace std {
template <> struct hash<elfin::ChainSeeker> {
    size_t operator()(const elfin::ChainSeeker & x) const {
        return hash<void*>()((void*) x.node) ^ x.chain_id;
    }
};
} /* std */

#endif  /* end of include guard: CHAIN_SEEKER_H_ */