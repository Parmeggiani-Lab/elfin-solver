#include "chain_seeker.h"

namespace elfin {

ChainSeeker::ChainSeeker(Node * _node, const size_t _chain_id) :
    node(_node), chain_id(_chain_id) {}

bool ChainSeeker::operator==(const ChainSeeker & other) const {
    return node == other.node and chain_id == other.chain_id;
}

std::string ChainSeeker::to_string() const {
    std::ostringstream ss;
    ss << "ChainSeeker[Node: 0x" << std::hex << node << std::dec;
    ss << ", Chain Id: " << chain_id << "]\n";
    return ss.str();
}

}  /* elfin */