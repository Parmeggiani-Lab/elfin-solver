#include "basic_node_team.h"

#include "jutil.h"
#include "basic_node_generator.h"
#include "kabsch.h"

namespace elfin {

/* protected */
/* modifiers */
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
/* ctors */
BasicNodeTeam::BasicNodeTeam(const BasicNodeTeam & other) {
    *this = other;
}

BasicNodeTeam * BasicNodeTeam::clone() const {
    return new BasicNodeTeam(*this);
}

/* accessors */
float BasicNodeTeam::score(const WorkArea * wa) const {
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
        score = new_score < score ? new_score : score;
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

/* modifiers */
BasicNodeTeam & BasicNodeTeam::operator=(const BasicNodeTeam & other) {
    if (this != &other) {
        disperse();
        deep_copy_from(&other);
    }
    return *this;
}

void BasicNodeTeam::remove_leaf_member(const Node * leaf_member) {
    NICE_PANIC(leaf_member->neighbors().size() != 1);
    const Link & link = leaf_member->neighbors().at(0);

    // Copy FreeChain for restore later
    const FreeChain chain_to_restore = link.dst();

    // Remove link to sever_point from the new tip node
    Node * new_tip = chain_to_restore.node;
    new_tip->remove_link(Link::reverse(link));

    // Restore FreeChain
    free_chains_.push_back(chain_to_restore);

    remove_member(leaf_member);
}

void BasicNodeTeam::destroy_limb(const Link arrow) {
    // Copy FreeChain for restore later
    const FreeChain chain_to_restore = arrow.dst();

    // Disconnect sever_point and new tip
    Link::sever(arrow);

    // Destroy limb
    BasicNodeGenerator<Node> bng = BasicNodeGenerator<Node>(arrow.src().node);
    while (not bng.is_done()) {
        remove_member(bng.next());
    }

    // Restore FreeChain
    free_chains_.push_back(chain_to_restore);
}

/* printers */
std::string BasicNodeTeam::to_string() const {
    std::ostringstream ss;

    NICE_PANIC(free_chains_.empty());
    Node * start_node = free_chains_.at(0).node;
    BasicNodeGenerator<Node> bng = BasicNodeGenerator<Node>(start_node);

    while (not bng.is_done()) {
        ss << bng.next()->to_string() << '\n';
    }

    return ss.str();
}

StrList BasicNodeTeam::get_node_names() const {
    StrList res;

    NICE_PANIC(free_chains_.empty());
    Node * start_node = free_chains_.at(0).node;
    BasicNodeGenerator<Node> bng = BasicNodeGenerator<Node>(start_node);

    while (not bng.is_done()) {
        res.emplace_back(bng.next()->prototype()->name);
    }

    DEBUG(res.size() != size(),
          string_format("res.size()=%lu, size()=%lu\n",
                        res.size(), this->size()));

    return res;
}

}  /* elfin */