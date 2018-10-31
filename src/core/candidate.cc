#include "candidate.h"

#include <sstream>

namespace elfin {

std::string Candidate::Node::to_string(const IdNameMap & inm) const {
    std::stringstream ss;
    ss << "ID: " << id_;
    ss << " Name: " << inm.at(id_);
    ss << " CoM: " << com_.to_string();
    return ss.str();
}

std::string Candidate::Node::to_csv_string() const {
    return com_.to_csv_string();
}

// virtual
std::string Candidate::to_string(const IdNameMap & inm) const {
    std::stringstream ss;

    const long N = nodes_.size();
    long i = 0;
    for (auto & n : nodes_)
    {
        i++;
        ss << "Node #" << i << " / " << N;
        ss << ": " << n.to_string(inm) << std::endl;
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
        const Point3f & pt = n.com_;
        checksum_cascade(&crc, &pt, sizeof(pt));
    }

    return crc;
}

}  /* elfin */