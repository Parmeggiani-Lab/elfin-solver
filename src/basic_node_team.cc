#include "basic_node_team.h"

#include "jutil.h"
#include "basic_node_generator.h"
#include "kabsch.h"

namespace elfin {

/* protected */
void BasicNodeTeam::deep_copy_from(
    const NodeTeam * other) {
    // Clone nodes (heap) and create address mapping
    std::vector<Node *> new_nodes;
    NodeAddrMap addr_map; // old addr -> new addr

    for (auto node_ptr : other->nodes()) {
        new_nodes.push_back(node_ptr->clone());
        addr_map[node_ptr] = new_nodes.back();
    }

    free_chains_ = other->free_chains();

    // Fix pointer addresses and assign to my own nodes
    for (auto node_ptr : new_nodes) {
        node_ptr->update_neighbor_ptrs(addr_map);
        nodes_.push_back(node_ptr);
    }

    for (auto & fc : free_chains_) {
        fc.node = addr_map.at(fc.node);
    }
}

/* public */
BasicNodeTeam::BasicNodeTeam(const BasicNodeTeam & other) {
    *this = other;
}

BasicNodeTeam * BasicNodeTeam::clone() const {
    return new BasicNodeTeam(*this);
}

float BasicNodeTeam::score(const WorkArea & wa) const {
    /*
     * In a BasicNodeTeam there are 2 and only 2 tips at all times. The nodes
     * network is thus a simple path. We walk the path to collect the 3D
     * points in order.
     *
     * In addition, we walk the path forward and backward, because the Kabsch
     * algorithm relies on point-wise correspondance. Different ordering can
     * yield different RMSD scores.
     */
    DEBUG(free_chains_.size() != 2);

    float score = INFINITY;
    for (auto & free_chain : free_chains_) {
        Node * tip = free_chain.node;
        BasicNodeGenerator<Node> bng = BasicNodeGenerator<Node>(tip);

        V3fList points;
        while (not bng.is_done()) {
            points.push_back(bng.next()->tx().collapsed());
        }

        DEBUG(points.size() != size(),
              string_format("points.size()=%lu, size()=%lu\n",
                            points.size(), this->size()));

        const float new_score = kabsch_score(points, wa);
        score = std::min(score, new_score);
    }

    return score;
}

Crc32 BasicNodeTeam::checksum() const {
    /*
     * We want the same checksum for two node teams that consist of the same
     * sequence of nodes even if they are in reverse order. This can be
     * achieved by XOR'ing the forward and backward checksums.
     */
    DEBUG(free_chains_.size() != 2);

    Crc32 crc = 0x0000;
    for (auto & free_chain : free_chains_) {
        Node * tip = free_chain.node;
        BasicNodeGenerator<Node> bng = BasicNodeGenerator<Node>(tip);

        Crc32 crc_half = 0xffff;
        while (not bng.is_done()) {
            const Node * node = bng.next();
            const ProtoModule * prot = node->prototype();
            checksum_cascade(&crc_half, &prot, sizeof(prot));
        }
        crc ^= crc_half;
    }

    return crc;
}

BasicNodeTeam & BasicNodeTeam::operator=(const BasicNodeTeam & other) {
    if (this != &other) {
        disperse();
        deep_copy_from(&other);
    }
    return *this;
}

StrList BasicNodeTeam::get_node_names() const {
    StrList res;

    Node * rand_tip = random_free_chain().node;
    BasicNodeGenerator<Node> bng = BasicNodeGenerator<Node>(rand_tip);

    while (not bng.is_done()) {
        res.emplace_back(bng.next()->prototype()->name);
    }

    DEBUG(res.size() != size(),
          string_format("res.size()=%lu, size()=%lu\n",
                        res.size(), this->size()));

    return res;
}

}  /* elfin */