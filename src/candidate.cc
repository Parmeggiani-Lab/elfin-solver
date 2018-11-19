#include "candidate.h"

#include <sstream>

namespace elfin {

StrList Candidate::get_node_names() const {
    StrList res;

    for (auto & n : nodes_) {
        res.emplace_back(n.prototype->name_);
    }

    return res;
}

std::string Candidate::Node::to_string() const {
    std::stringstream ss;
    ss << "Name: " << prototype->name_;
    ss << ", CoM: " << com.to_string();
    return ss.str();
}

std::string Candidate::Node::to_csv_string() const {
    return com.to_csv_string();
}

// virtual
std::string Candidate::to_string() const {
    std::stringstream ss;

    const size_t N = nodes_.size();
    for (size_t i = 0; i < N; ++i)
    {
        const Node const & n = nodes_[i];
        ss << "Nodes[#" << (i + 1) << " / " << N << "]: ";
        ss << n.to_string() << std::endl;
    }

    return ss.str();
}

// virtual
std::string Candidate::to_csv_string() const {
    std::stringstream ss;

    for (auto & n : nodes_) {
        ss << n.to_csv_string() << std::endl;
    }

    return ss.str();
}

Crc32 Candidate::checksum() const
{
    // Compute checksum lazily because it's only used once per generation
    Crc32 crc = 0xffff;
    for (auto & n : nodes_) {
        const Vector3f & pt = n.com;
        checksum_cascade(&crc, &pt, sizeof(pt));
    }

    return crc;
}

}  /* elfin */