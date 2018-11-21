#include "candidate.h"

#include <sstream>

#include "random_utils.h"

namespace elfin {

StrList Candidate::get_node_names() const {
    StrList res;

    for (auto & n : nodes_) {
        res.emplace_back(n.prototype->name_);
    }

    return res;
}

// virtual
std::string Candidate::to_string() const {
    std::stringstream ss;

    const size_t N = nodes_.size();
    for (size_t i = 0; i < N; ++i)
    {
        const Node & n = nodes_[i];
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
        const Transform & pt = n.tx;
        checksum_cascade(&crc, &pt, sizeof(pt));
    }

    return crc;
}

bool Candidate::PtrComparator(const Candidate * lhs, const Candidate * rhs) {
    return lhs->get_score() < rhs->get_score();
};

}  /* elfin */