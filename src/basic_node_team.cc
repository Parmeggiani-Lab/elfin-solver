#include "basic_node_team.h"

#include "jutil.h"
#include "basic_node_generator.h"
#include "random_utils.h"
#include "kabsch.h"
#include "input_manager.h"
#include "id_types.h"

// #define NO_LIMB_MUTATE
#define NO_POINT_MUTATE
#define NO_CROSS_MUTATE

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

void BasicNodeTeam::remove_leaf_member(Node * side_member) {
    NICE_PANIC(side_member->neighbors().size() != 1);
    const Link & link = side_member->neighbors().at(0);

    // Copy FreeChain for restore later
    const FreeChain chain_to_restore = link.dst();

    // Remove link to sever_point from the new tip node
    Node * new_tip = chain_to_restore.node;
    new_tip->remove_link(Link::reverse(link));

    // Restore FreeChain
    free_chains_.push_back(chain_to_restore);

    remove_member(side_member);
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

/*
 * Compute index pairs of mother and father nodes at which cross mutation is
 * possible.
 */
IdPairs get_crossing_ids(
    const NodeTeam & mother,
    const NodeTeam & father) {
    IdPairs crossing_ids;

#ifndef NO_CROSS_MUTATE
    const size_t mn_len = mother.size();;
    const size_t fn_len = father.size();

    for (long i = 1; i < (long) mn_len - 1; i++) {
        // Using i as nodes1 left limb cutoff
        for (long j = 1; j < (long) fn_len - 1; j++) {
            if (mother.at(i)->prototype() == father.at(j)->prototype()) {
                crossing_ids.push_back(IdPair(i, j));
            }
        }
    }
#endif // NO_CROSS_MUTATE

    return crossing_ids;
}

bool BasicNodeTeam::limb_mutate() {
    bool mutate_success = false;

#ifndef NO_LIMB_MUTATE
    // Server limb
    Node * sever_point = nodes_.rand_item();

    const size_t neighbor_size = sever_point->neighbors().size();
    if (neighbor_size == 1) {
        // Never try to delete the whole team! Instead, delete this side node.
        remove_leaf_member(sever_point);
    }
    else if (neighbor_size == 2) {
        // Pick surviving limb
        destroy_limb(sever_point->neighbors().rand_item());
    }
    else {
        NICE_PANIC(
            "neighbor_size",
            string_format("Strange neighbor size: %lu\n", neighbor_size));
    }

    // Re-generate
    grow(random_free_chain());

    mutate_success = true;
#endif // NO_LIMB_MUTATE

    return mutate_success;
}

bool BasicNodeTeam::point_mutate() {
    bool mutate_success = false;

#ifndef NO_POINT_MUTATE
    const size_t n_nodes = nodes_.size();
    const size_t rm_dim = REL_MAT.size();
    std::vector<PointMutateMode> modes =
    {
        PointMutateMode::SwapMode,
        PointMutateMode::InsertMode,
        PointMutateMode::DeleteMode
    };

    while (!mutate_success and !modes.empty()) {
        // Draw a random mode without replacement
        const PointMutateMode pm_mode = pop_random(modes);

        // Try to perform point_mutate in the chosen mode
        switch (pm_mode) {
        case PointMutateMode::SwapMode: {
            // First int is index from nodes_
            // Second int is ido swap to
            IdPairs swappable_ids;
            for (size_t i = 0; i < n_nodes; i++) {
                // For all neighbours of previous node
                // find those that has nodes[i+1] as
                // one of their RHS neighbours
                for (size_t j = 0; j < rm_dim; j++) {
                    // Make sure it's not the original one
                    if (j != nodes_.at(i).id) {
                        // Check whether i can be exchanged for j
                        if ((i == 0 or REL_MAT.at(nodes_.at(i - 1).id).at(j))
                                and
                                (i == n_nodes - 1 or REL_MAT.at(j).at(nodes_.at(i + 1).id))
                           ) {
                            // Make sure resultant shape won't collide with itself
                            Nodes test_nodes(nodes_);
                            test_nodes.at(i).id = j;

                            // dbg("checking swap at %d/%d of %s\n",
                            //     i, n_nodes, to_string().c_str());
                            if (synthesise(test_nodes))
                                swappable_ids.push_back(IdPair(i, j));
                        }
                    }
                }
            }

            // // Pick a random one, or fall through to next case
            if (swappable_ids.size() > 0) {
                const IdPair & ids = pick_random(swappable_ids);
                nodes_.at(ids.x).id = ids.y;

                synthesise(nodes_); // This is guaranteed to succeed
                mutate_success = true;
            }
            break;
        }
        case PointMutateMode::InsertMode: {
            IdPairs insertable_ids;
            for (size_t i = 0; i < n_nodes; i++) {
                for (size_t j = 0; j < rm_dim; j++) {
                    // Check whether j can be inserted before i
                    if (
                        (i == 0 or // Pass if inserting at the left end
                         REL_MAT.at(nodes_.at(i - 1).id).at(j))
                        and
                        (i == n_nodes or // Pass if appending at the right end
                         REL_MAT.at(j).at(nodes_.at(i).id))
                    ) {
                        // Make sure resultant shape won't collide with itself
                        Nodes test_nodes(nodes_);
                        Node new_node;
                        new_node.id = j;
                        test_nodes.insert(test_nodes.begin() + i, //This is insertion before i
                                          new_node);

                        // dbg("checking insertion at %d/%d of %s\n",
                        //     i, n_nodes, to_string().c_str());
                        if (synthesise(test_nodes))
                            insertable_ids.push_back(IdPair(i, j));
                    }
                }
            }

            // Pick a random one, or fall through to next case
            if (insertable_ids.size() > 0) {
                const IdPair & ids = pick_random(insertable_ids);
                Node new_node;
                new_node.id = ids.y;
                nodes_.insert(nodes_.begin() + ids.x, //This is insertion before i
                              new_node);

                synthesise(nodes_); // This is guaranteed to succeed
                mutate_success = true;
            }
            break;
        }
        case PointMutateMode::DeleteMode: {
            std::vector<size_t> deletable_ids;
            if (n_nodes > 0) {
                deletable_ids.push_back(0);
                deletable_ids.push_back(n_nodes - 1);

                for (long i = 1; i < (long) n_nodes - 1; ++i) {
                    // Check whether i can be deleted
                    const Node & lhs_node = nodes_.at(i - 1);
                    const Node & rhs_node = nodes_.at(i + 1);
                    if (REL_MAT.at(lhs_node.id).at(rhs_node.id)) {
                        // Make sure resultant shape won't collide with itself
                        Nodes test_nodes(
                            nodes_.begin(),
                            nodes_.begin() + i);
                        test_nodes.insert(
                            test_nodes.end(),
                            nodes_.begin() + i + 1,
                            nodes_.end());

                        // dbg("checking deletion at %d/%d of %s\n",
                        //     i, n_nodes, to_string().c_str());
                        if (synthesise(test_nodes)) {
                            deletable_ids.push_back((size_t) i);
                        }
                    }
                }
                // Pick a random one, or report impossible
                if (!deletable_ids.empty()) {
                    const size_t delete_idx = pick_random(deletable_ids);
                    nodes_.erase(nodes_.begin() + delete_idx);
                    synthesise(nodes_); // This is guaranteed to succeed
                    mutate_success = true;
                }
            }
            break;
        }
        default: {
            // Fell through all cases without mutating
            // Do nothing unless pm_mode is strange
            NICE_PANIC(pm_mode < 0 or pm_mode >= PointMutateMode::EnumSize,
                       "Invalid pm_mode in Chromosome::pointMutate()\n");
        }
        } // end of pm_mode switch
    }
#endif // NO_POINT_MUTATE

    return mutate_success;
}

bool BasicNodeTeam::cross_mutate(
    const NodeTeam * mother,
    const NodeTeam * father) {
    bool cross_ok = false;

#ifndef NO_CROSS_MUTATE
    const Nodes & mother_nodes = mother->nodes();
    const Nodes & father_nodes = father->nodes();
    IdPairs crossing_ids = get_crossing_ids(mother_nodes, father_nodes);

    size_t tries = 0;
    // cross can fail - if resultant nodes collide during synth
    while (tries < MAX_FREECANDIDATE_MUTATE_FAILS and
            crossing_ids.size()) {
        // Pick random crossing point
        const IdPair & cross_point = pick_random(crossing_ids);

        Nodes new_nodes;

        new_nodes.insert(
            new_nodes.end(),
            mother_nodes.begin(),
            mother_nodes.begin() + cross_point.x);

        new_nodes.insert(
            new_nodes.end(),
            father_nodes.begin() + cross_point.y,
            father_nodes.end());

        die("Collision check here?\n");
        if (synthesise(new_nodes)) {
            nodes_ = new_nodes;
            cross_ok = true;
            break;
        }

        tries++;
    }
#endif // NO_CROSS_MUTATE

    return cross_ok;
}

void BasicNodeTeam::grow(FreeChain free_chain) {
    while (size() < Candidate::MAX_LEN) {
        const ProtoLink & pt_link = random_proto_link(free_chain);
        invite_new_member(free_chain, pt_link);

        // Pick next tip chain
        free_chain = random_free_chain();
    }
}

void BasicNodeTeam::regrow() {
    remake(XDB.basic_mods());
    grow(random_free_chain());
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