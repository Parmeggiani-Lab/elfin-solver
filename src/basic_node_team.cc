#include "basic_node_team.h"

#include "jutil.h"
#include "basic_node_generator.h"
#include "kabsch.h"
#include "input_manager.h"
#include "id_types.h"

// #define NO_ERODE
// #define NO_DELETE
#define NO_INSERT
#define NO_SWAP
#define NO_CROSS
// #define NO_RANDOMIZE

#if defined(NO_ERODE) || \
defined(NO_DELETE) || \
defined(NO_INSERT) || \
defined(NO_SWAP) || \
defined(NO_CROSS) || \
defined(NO_RANDOMIZE)
#warning "At least one mutation function is DISABLED!!"
#endif

// #define SHOW_UNDELETABLE

namespace elfin {

/* private */
/*modifiers */

void BasicNodeTeam::fix_limb_transforms(const Link & arrow) {
    BasicNodeGenerator limb_gtor(&arrow);
    while (not limb_gtor.is_done()) {
        Node * prev_node = limb_gtor.curr_node();

        const Link * link = limb_gtor.curr_link();
        DEBUG(link == nullptr);

        Node * curr_node = limb_gtor.next(); // This modifies limb_gtor.curr_link()
        const ProtoLink * proto_link =
            prev_node->prototype()->find_link_to(
                link->src().chain_id,
                link->src().term,
                link->dst().node->prototype(),
                link->dst().chain_id);

        DEBUG(proto_link == nullptr);
        curr_node->tx_ =  proto_link->tx * curr_node->tx_;
    }
}

bool BasicNodeTeam::erode_mutate() {
    bool mutate_success = false;

#ifndef NO_ERODE
    if (not free_chains_.empty()) {
        // Pick random tip node
        Node * tip_node = free_chains_.pick_random().node;

        // Remove all free chains originating from tip_node
        remove_member_chains(tip_node);

        float p = 1.0f; // probability
        FreeChain chain_to_restore;

        // Loop condition is always true on first entrance, hence do-while.
        do {
            // Make new tip node state consistent
            NICE_PANIC(tip_node->neighbors().size() != 1);
            const Link tip_link = tip_node->neighbors().at(0);

            chain_to_restore = tip_link.dst();
            Node * new_tip = chain_to_restore.node;
            //               (  tip_link  )
            // X--[tip_node]--src->-<-dst
            // chain_to_restore:      ^^^
            //
            //                dst->-<-src--[new_tip]--[]--[]--...

            remove_member(tip_node);
            //               (  tip_link  )
            // {          freed          }
            //
            //                dst->-<-src--[new_tip]--[]--[]--...

            // Remove tip_link to tip_node from the new_tip
            new_tip->remove_link(tip_link.reversed());
            //               (  tip_link  )
            // {          freed          }
            //
            //                          X--[new_tip]--[]--[]--...

            // Update tip node ptr
            tip_node = new_tip;

            /*
             * Calculate next probability. The following formula gives:
             * remaining size / original size; p
             * 6/6; p=0.8333333333333334
             * 5/6; p=0.6666666666666667
             * 4/6; p=0.5
             * 3/6; p=0.3333333333333333
             * 2/6; p=0.16666666666666666
             * 1/6; p=0.0
             */
            p = p * (size() - 1) / size();
        } while (random::get_dice_0to1() <= p);

        // Restore FreeChain
        free_chains_.push_back(chain_to_restore);

        regenerate();

        mutate_success = true;
    }
#endif // NO_ERODE

    return mutate_success;
}

bool BasicNodeTeam::delete_mutate() {
    bool mutate_success = false;

#ifndef NO_DELETE
    if (not free_chains_.empty()) {
        // Collect deletable nodes
        Vector<Node *> deletable_nodes;

        Node * tip = free_chains_[0].node; // starting at either end is fine
        BasicNodeGenerator node_gtor(tip);
        node_gtor.next(); // skip start tip

        Node * curr_node = node_gtor.next();
        Node * next_node = node_gtor.next();
        while (next_node) { // skip end tip too
            DEBUG(curr_node->neighbors().size() != 2);

            const FreeChain & fchain1 = curr_node->neighbors().at(0).dst();
            const FreeChain & fchain2 = curr_node->neighbors().at(1).dst();

            // X--[neighbor1]--src->-<-dst               dst->-<-src--[neighbor2]--...
            //              (fchain1)                         (fchain2)
            //                 vvv                               vvv
            //                 dst->-<-src--[curr_node]--src->-<-dst
            const bool deletable =
                fchain1.node->prototype()->has_link_to(
                    fchain1.term,
                    fchain2.node->prototype(),
                    fchain2.chain_id);
            // The reverse doesn't need to be checked, because all links have
            // a reverse

            if (deletable) {
                deletable_nodes.push_back(curr_node);
            }

            curr_node = next_node;
            next_node = node_gtor.next();
        }

        if (not deletable_nodes.empty()) {
            // Delete a random one
            Node * to_delete = deletable_nodes.pick_random();

            // Link up neighbor1 and neighbor2
            // X--[neighbor1]--src->-<-dst               dst->-<-src--[neighbor2]--...
            //                 (  link1  )               (  link2  )
            //                 vvvvvvvvvvv               vvvvvvvvvvv
            //                 dst->-<-src--[curr_node]--src->-<-dst
            const Link & link1 = to_delete->neighbors().at(0);
            const Link & link2 = to_delete->neighbors().at(1);

            Node * neighbor1 = link1.dst().node;
            Node * neighbor2 = link2.dst().node;

            neighbor1->remove_link(link1.reversed());
            neighbor2->remove_link(link2.reversed());
            // X--[neighbor1]--X                                   X--[neighbor2]--...
            //                 (  link1  )               (  link2  )
            //                 vvvvvvvvvvv               vvvvvvvvvvv
            //                 dst->-<-src--[curr_node]--src->-<-dst

            // Create links between neighbor1 and neighbor2
            const Link arrow1(link1.dst(), link2.dst());
            neighbor1->add_link(arrow1.src(), arrow1.dst());
            neighbor2->add_link(link2.dst(), link1.dst());
            //           link1.dst(), link2.dst()
            //                 vvv     vvv
            // X--[neighbor1]--src->-<-dst
            //                 dst->-<-src--[neighbor2]--...
            //                 ^^^     ^^^
            //           link1.dst(), link2.dst()

            // From this point on, memory of link1 and link2 are invalid!!!
            remove_member(to_delete);

            // to_delete is guranteed to not be a tip so no need to clean up free_chains_

            fix_limb_transforms(arrow1);

            mutate_success = true;
        }
        else {
#ifdef SHOW_UNDELETABLE
            wrn("\nNo deletable nodes! checksum=%x\n%s\n",
                checksum(),
                to_string().c_str());
#endif  /* ifdef SHOW_UNDELETABLE */
        }
    }
#endif

    return mutate_success;
}

bool BasicNodeTeam::insert_mutate() {
    bool mutate_success = false;

#ifndef NO_INSERT
    if (not free_chains_.empty()) {
        // TODO

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

        //
    }
#endif

    return mutate_success;
}

bool BasicNodeTeam::swap_mutate() {
    bool mutate_success = false;

#ifndef NO_SWAP
    if (not free_chains_.empty()) {
        // TODO

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

        //
    }
#endif

    return mutate_success;
}


/*
 * Compute index pairs of mother and father nodes at which cross mutation is
 * possible.
 */
#ifndef NO_CROSS
IdPairs get_crossing_ids(
    const NodeTeam & mother,
    const NodeTeam & father) {
    IdPairs crossing_ids;

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

    return crossing_ids;
}
#endif // NO_CROSS

bool BasicNodeTeam::cross_mutate(
    const NodeTeam * father) {
    bool mutate_success = false;

#ifndef NO_CROSS
    DEBUG(size() == 0);

    const Nodes & mother_nodes = nodes_; // Self has already inherited mother
    const Nodes & father_nodes = father->nodes_;
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
#endif

    return mutate_success;
}

bool BasicNodeTeam::regenerate() {
    if (nodes_.empty()) {
        // Pick random initial member
        add_member(XDB.basic_mods().draw());
    }

    DEBUG(free_chains_.empty());

    FreeChain free_chain = free_chains_.pick_random();
    while (size() < Candidate::MAX_LEN) {
        const ProtoLink & pt_link = random_proto_link(free_chain);
        invite_new_member(free_chain, pt_link);

        // Pick next tip chain
        free_chain = free_chains_.pick_random();
    }

    return true;
}

bool BasicNodeTeam::randomize_mutate() {
    bool mutate_success = false;

#ifndef NO_RANDOMIZE
    disperse();
    add_member(XDB.basic_mods().draw());
    regenerate();

    mutate_success = true;
#endif

    return mutate_success;
}


/* public */
/* ctors */
BasicNodeTeam::BasicNodeTeam(const BasicNodeTeam & other) {
    deep_copy_from(&other);
}

BasicNodeTeam * BasicNodeTeam::clone() const {
    return new BasicNodeTeam(*this);
}

/* accessors */
float BasicNodeTeam::score(const WorkArea * wa) const {
    /*
     * In a BasicNodeTeam there are either 0 or 2 tips at any given time. The
     * nodes network is thus a simple path. We walk the path to collect the 3D
     * points in order.
     *
     * In addition, we walk the path forward and backward, because the Kabsch
     * algorithm relies on point-wise correspondance. Different ordering can
     * yield different RMSD scores.
     */
    DEBUG(free_chains_.size() != 2 and size() != 0);

    float score = INFINITY;
    for (auto & free_chain : free_chains_) {
        Node * tip = free_chain.node;
        BasicNodeGenerator node_gtor(tip);

        V3fList points;
        while (not node_gtor.is_done()) {
            points.push_back(node_gtor.next()->tx_.collapsed());
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
    DEBUG(free_chains_.size() != 2 and size() != 0);

    Crc32 crc = 0x0000;
    for (auto & free_chain : free_chains_) {
        Node * tip = free_chain.node;
        BasicNodeGenerator node_gtor(tip);

        Crc32 crc_half = 0xffff;
        while (not node_gtor.is_done()) {
            const Node * node = node_gtor.next();
            const ProtoModule * prot = node->prototype();
            checksum_cascade(&crc_half, &prot, sizeof(prot));
        }
        crc ^= crc_half;
    }

    return crc;
}

/* modifiers */
void BasicNodeTeam::deep_copy_from(
    const NodeTeam * other) {
    if (this != other) {
        disperse();

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
}

MutationMode BasicNodeTeam::mutate(
    const NodeTeam * mother,
    const NodeTeam * father) {
    // Inherit from mother
    deep_copy_from(mother);

    MutationModeList modes = gen_mutation_mode_list();

    bool mutation_ok = false;
    MutationMode mode;

    while (not mutation_ok and not modes.empty()) {
        mode = modes.pop_random();
        switch (mode) {
        case MutationMode::ERODE:
            mutation_ok = erode_mutate();
            break;
        case MutationMode::DELETE:
            mutation_ok = delete_mutate();
            break;
        case MutationMode::INSERT:
            mutation_ok = insert_mutate();
            break;
        case MutationMode::SWAP:
            mutation_ok = swap_mutate();
            break;
        case MutationMode::CROSS:
            mutation_ok = cross_mutate(father);
            break;
        case MutationMode::REGENERATE:
            mutation_ok = regenerate();
            break;
        case MutationMode::RANDOMIZE:
            mutation_ok = randomize_mutate();
            break;
        default:
            bad_mutation_mode(mode);
        }
    }

    return mode;
}

/* printers */
std::string BasicNodeTeam::to_string() const {
    std::ostringstream ss;

    NICE_PANIC(free_chains_.empty());
    Node * start_node = free_chains_.at(0).node;
    BasicNodeGenerator node_gtor(start_node);

    while (not node_gtor.is_done()) {
        ss << node_gtor.next()->to_string();
        const Link * link_ptr = node_gtor.curr_link();
        if (link_ptr) {
            const TerminusType src_term = link_ptr->src().term;
            const TerminusType dst_term = link_ptr->dst().term;
            ss << "\n(" << TerminusTypeToCStr(src_term) << ", ";
            ss << TerminusTypeToCStr(dst_term) << ")\n";
        }
    }

    return ss.str();
}

StrList BasicNodeTeam::get_node_names() const {
    StrList res;

    NICE_PANIC(free_chains_.empty());
    Node * start_node = free_chains_.at(0).node;
    BasicNodeGenerator node_gtor(start_node);

    while (not node_gtor.is_done()) {
        res.emplace_back(node_gtor.next()->prototype()->name);
    }

    DEBUG(res.size() != size(),
          string_format("res.size()=%lu, size()=%lu\n",
                        res.size(), this->size()));

    return res;
}

}  /* elfin */