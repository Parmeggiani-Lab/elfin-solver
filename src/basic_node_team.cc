#include "basic_node_team.h"

#include <unordered_map>

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
     */
    Node * rand_tip = random_free_chain().node;
    BasicNodeGenerator<Node> bng = BasicNodeGenerator<Node>(rand_tip);

    V3fList points;
    while (not bng.is_done()) {
        points.push_back(bng.next()->tx().collapsed());
    }

    DEBUG(points.size() != size(),
          string_format("points.size()=%lu, size()=%lu\n",
                        points.size(), this->size()));

    return kabsch_score(points, wa);
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