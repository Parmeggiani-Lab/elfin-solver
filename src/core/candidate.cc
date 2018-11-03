#include "candidate.h"

#include <sstream>

#include "shorthands.h"

namespace elfin {

const std::vector<std::string> Candidate::get_node_names() const {
    std::vector<std::string> res;

    for (auto & n : nodes_)
        res.emplace_back(ID_NAME_MAP.at(n.id));

    return res;
}

std::string Candidate::Node::to_string() const {
    std::stringstream ss;
    ss << "ID: " << id;
    ss << " Name: " << ID_NAME_MAP.at(id);
    ss << " CoM: " << com.to_string();
    return ss.str();
}

std::string Candidate::Node::to_csv_string() const {
    return com.to_csv_string();
}

// virtual
std::string Candidate::to_string() const {
    std::stringstream ss;

    const long N = nodes_.size();
    long i = 0;
    for (auto & n : nodes_)
    {
        i++;
        ss << "Node #" << i << " / " << N;
        ss << ": " << n.to_string() << std::endl;
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
        const Point3f & pt = n.com;
        checksum_cascade(&crc, &pt, sizeof(pt));
    }

    return crc;
}

}  /* elfin */