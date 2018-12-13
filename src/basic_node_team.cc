#include "basic_node_team.h"

#include "jutil.h"
#include "basic_node_generator.h"
#include "kabsch.h"
#include "input_manager.h"
#include "id_types.h"

// #define NO_ERODE
// #define NO_DELETE
// #define NO_INSERT
// #define NO_SWAP
#define NO_CROSS
// #define NO_REGENERATE

#if defined(NO_ERODE) || \
defined(NO_DELETE) || \
defined(NO_INSERT) || \
defined(NO_SWAP) || \
defined(NO_CROSS) || \
defined(NO_REGENERATE)
#warning "At least one mutation function is DISABLED!!"
#endif

// #define SHOW_UNDELETABLE

namespace elfin {

/* private */
/* accessors */
Crc32 BasicNodeTeam::calc_checksum() const {
    /*
     * We want the same checksum for two node teams that consist of the same
     * sequence of nodes even if they are in reverse order. This can be
     * achieved by XOR'ing the forward and backward checksums.
     */
    DEBUG(free_chains_.size() != 2 and size() != 0);

    Crc32 crc = 0x0000;
    for (auto& free_chain : free_chains_) {
        Node* tip = free_chain.node;
        BasicNodeGenerator node_gtor(tip);

        Crc32 crc_half = 0xffff;
        while (not node_gtor.is_done()) {
            Node const* node = node_gtor.next();
            ProtoModule const* prot = node->prototype();
            checksum_cascade(&crc_half, &prot, sizeof(prot));
        }

        if (crc ^ crc_half) {
            // If result is 0x0000 then it's due to the node sequence being
            // symmetrical.
            crc ^= crc_half;
        }
    }

    return crc;
}

float BasicNodeTeam::calc_score(WorkArea const* wa) const {
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
    for (auto& free_chain : free_chains_) {
        Node* tip = free_chain.node;
        BasicNodeGenerator node_gtor(tip);

        V3fList points;
        while (not node_gtor.is_done()) {
            points.push_back(node_gtor.next()->tx_.collapsed());
        }

        DEBUG(points.size() != size(),
              string_format("points.size()=%lu, size()=%lu\n",
                            points.size(), this->size()));

        float const new_score = kabsch_score(points, wa);
        score = new_score < score ? new_score : score;
    }

    return score;
}

/*modifiers */
void BasicNodeTeam::fix_limb_transforms(Link const& arrow) {
    BasicNodeGenerator limb_gtor(&arrow);
    while (not limb_gtor.is_done()) {
        Link const* curr_link = limb_gtor.curr_link();
        Node* curr_node = limb_gtor.curr_node();
        Node* next_node = limb_gtor.next();
        next_node->tx_ = curr_node->tx_ * curr_link->prototype()->tx();
    }
}

void BasicNodeTeam::grow_tip(FreeChain free_chain_a) {
    ProtoLink const& proto_link =
        free_chain_a.random_proto_link();

    Node* node_a = free_chain_a.node;
    Node* node_b = add_member(
                       proto_link.module(),
                       node_a->tx_ * proto_link.tx());

    TerminusType const term_a = free_chain_a.term;
    TerminusType const term_b = opposite_term(term_a);

    FreeChain const free_chain_b =
        FreeChain(node_b, term_b, proto_link.chain_id());

    node_a->add_link(free_chain_a, &proto_link, free_chain_b);
    node_b->add_link(free_chain_b, proto_link.reverse(), free_chain_a);

    free_chains_.lift_erase(free_chain_a);
    free_chains_.lift_erase(free_chain_b);
}

bool BasicNodeTeam::erode_mutate(
    Node* tip_node,
    long stop_after_n,
    bool const regen) {
    bool mutate_success = false;

#ifndef NO_ERODE
    stop_after_n = std::min(stop_after_n, (long) size());
    // Any stop_after_n < 0 indicates to continue forever by linearly falling
    // probability.

    if (stop_after_n == 0) {
        mutate_success = true; // eroded 0 node as requested... ?
    }
    else if (not free_chains_.empty()) {
        // free_chains_ should not be empty even if tip_node is specified.

        // Pick random tip node if not specified
        if (tip_node == nullptr) {
            FreeChain const& fc = free_chains_.pick_random();
            tip_node = fc.node;
        }

        // Remove all free chains originating from tip_node
        remove_member_chains(tip_node);

        float p = 1.0f; // probability
        FreeChain chain_to_restore;

        // Loop condition is always true on first entrance, hence do-while.
        do {
            DEBUG(size() == 0);

            // A tip node is guranteed to have only one neighbor.
            const size_t num_links = tip_node->links().size();
            NICE_PANIC(num_links != 1);
            Link const tip_link = tip_node->links().at(0);

            // Delete this tip node and restore state to consistency
            chain_to_restore = tip_link.dst();
            Node* new_tip = chain_to_restore.node;

            /*
             *                 ( tip_link )
             *  X--[tip_node]--src->-<-dst
             *  chain_to_restore:      ^^^
             *                 dst->-<-src--[new_tip]--[]--[]--...
             */
            remove_member(tip_node);

            /*
             *                 ( tip_link )
             *  {          freed          }
             *                 dst->-<-src--[new_tip]--[]--[]--...
             */
            // Remove tip_link to tip_node from the new_tip. Because it's
            // reversed, use dst instead of src.
            new_tip->remove_link(tip_link.dst());

            /*
             *                (  tip_link  )
             *  {          freed          }
             *                           X--[new_tip]--[]--[]--...
             */
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

            // stop_after_n < 0 < stop_after_n is true
            stop_after_n--;

            /*
             * Continue if:
             * (stop_after_n > 0 or stop_after_n < 0) ... i.e.
             * stop_after_n != 0
             * and
             * random::get_dice_0to1() <= p
             */
        } while (stop_after_n != 0 and random::get_dice_0to1() <= p);

        // Restore FreeChain
        free_chains_.push_back(chain_to_restore);

        if (regen) regenerate();

        mutate_success = true;
    }
#endif // NO_ERODE

    return mutate_success;
}

struct DeletePoint {
    /*
       [neighbor1] <--link1-- [delete_node] --link2--> [neighbor2]
                   --------------skipper------------->
    */
    Node* delete_node;
    Link const* link1, * link2;
    ProtoLink const* skipper;
    DeletePoint(
        Node* _delete_node,
        Link const* _link1,
        Link const* _link2,
        ProtoLink const* _skipper) :
        link1(_link1),
        link2(_link2),
        delete_node(_delete_node),
        skipper(_skipper) {}
};

bool BasicNodeTeam::delete_mutate() {
    bool mutate_success = false;

#ifndef NO_DELETE
    if (not free_chains_.empty()) {
        // Walk through all nodes to collect delete points.
        Vector<DeletePoint> delete_points;

        // Starting at either end is fine.
        DEBUG(free_chains_.size() != 2);
        Node* start_node = free_chains_[0].node;
        BasicNodeGenerator node_gtor(start_node);

        Node* curr_node = nullptr;
        Node* next_node = node_gtor.next(); // starts with start_node
        do {
            curr_node = next_node;
            next_node = node_gtor.next(); // can be nullptr
            size_t const num_links = curr_node->links().size();

            if (num_links == 1) {
                /*
                 * curr_node is a tip node, which can always be deleted trivially.
                 * Use ProtoLink* = nullptr to mark a tip node. Pointers
                 * link1 and link2 are not used.
                 */

                delete_points.emplace_back(
                    curr_node, // delete_node
                    nullptr, // link1
                    nullptr, // link2
                    nullptr); // skipper
            }
            else if (num_links == 2) {
                /*
                 * curr_node is between start and end node. Find a link that
                 * skips curr_node. The reverse doesn't need to be checked,
                 * because all links have a reverse.
                 */

                Link const& link1 = curr_node->links().at(0);
                Link const& link2 = curr_node->links().at(1);
                FreeChain const& fchain1 = link1.dst();
                FreeChain const& fchain2 = link2.dst();

                /*
                 * X--[neighbor1]--src->-<-dst               dst->-<-src--[neighbor2]--...
                 *                 (  link1  )               (  link2  )
                 *                 vvvvvvvvvvv               vvvvvvvvvvv
                 *              (fchain1)                         (fchain2)
                 *                 vvv                               vvv
                 *                 dst->-<-src--[curr_node]--src->-<-dst
                */
                ProtoLink const* const proto_link_ptr =
                    fchain1.node->prototype()->find_link_to(
                        fchain1.chain_id,
                        fchain1.term,
                        fchain2.node->prototype(),
                        fchain2.chain_id);

                if (proto_link_ptr) {
                    delete_points.emplace_back(
                        curr_node, // delete_node
                        &link1, // link1
                        &link2, // link2
                        proto_link_ptr); // skipper
                }
            }
            else {
                NICE_PANIC("Unexpected num_links",
                           string_format(
                               "Number of links: %lu\n", num_links));
            }
        } while (not node_gtor.is_done());

        // delete_points will at least contain the tip nodes.
        DEBUG(delete_points.empty());

        // Delete a node using a random deletable point
        DeletePoint const& delete_point = delete_points.pick_random();
        if (delete_point.skipper) {
            /*
             *  This is NOT a tip node. Need to do some clean up
             *
             *  Link up neighbor1 and neighbor2
             *  X--[neighbor1]--src->-<-dst                 dst->-<-src--[neighbor2]--...
             *                  (  link1  )                 (  link2  )
             *                  vvvvvvvvvvv                 vvvvvvvvvvv
             *                  dst->-<-src--[delete_node]--src->-<-dst
             */
            Node* neighbor1 = delete_point.link1->dst().node;
            Node* neighbor2 = delete_point.link2->dst().node;

            neighbor1->remove_link(delete_point.link1->dst());
            neighbor2->remove_link(delete_point.link2->dst());
            /*
             *  X--[neighbor1]--X                                     X--[neighbor2]--...
             *                  (  link1  )                 (  link2  )
             *                  vvvvvvvvvvv                 vvvvvvvvvvv
             *                  dst->-<-src--[delete_node]--src->-<-dst
             *                  ----------------arrow1---------------->
             */

            // Create links between neighbor1 and neighbor2
            Link const arrow1(delete_point.link1->dst(),
                              delete_point.skipper,
                              delete_point.link2->dst());
            neighbor1->add_link(arrow1);
            neighbor2->add_link(arrow1.reversed());
            /*
             *         link1->dst()     link2->dst()
             *                  vvv     vvv
             *  X--[neighbor1]--src->-<-dst
             *                  dst->-<-src--[neighbor2]--...
             *                  ^^^     ^^^
             *         link1->dst()     link2->dst
             */

            // From this point on, memory of link1 and link2 are invalid!!!
            remove_member(delete_point.delete_node);

            // delete_node is guranteed to not be a tip so no need to clean up
            // free_chains_
            fix_limb_transforms(arrow1);
        }
        else {
            // This is a tip node. Deleting is trivial - same as erode_mutate().
            erode_mutate(delete_point.delete_node, 1, false);
        }

        mutate_success = true;
    }
#endif

    return mutate_success;
}

struct BridgePoint {
    /*
     *  [  node1 ] --------------link-------------> [ node2  ]
     *             src                          dst
     *
     *  Each bridge has ptlink1 and ptlink2 that:
     *  [  node1 ] -ptlink1-> [new_node] -ptlink2-> [ node2  ]
     */
    FreeChain const src, dst;
    FreeChain::BridgeList const bridges;
    BridgePoint(
        FreeChain const& _src,
        FreeChain const& _dst) :
        src(_src),
        dst(_dst),
        bridges(dst.node == nullptr ?
                FreeChain::BridgeList() :
                src.find_bridges(dst)) { }
};

bool BasicNodeTeam::insert_mutate() {
    bool mutate_success = false;

#ifndef NO_INSERT
    if (not free_chains_.empty()) {
        // Walk through all links to collect insert points.
        Vector<BridgePoint> insert_points;

        // Starting at either end is fine.
        DEBUG(free_chains_.size() != 2);
        Node* start_node = free_chains_[0].node;
        BasicNodeGenerator node_gtor(start_node);

        Node* curr_node = nullptr;
        Node* next_node = node_gtor.next(); // starts with start_node
        do {
            curr_node = next_node;
            next_node = node_gtor.next(); // can be nullptr
            size_t const num_links = curr_node->links().size();

            if (num_links == 1) {
                /*
                 *  curr_node is a tip node. A new node can be inserted on the
                 *  unconnected terminus of a tip node trivially. Use nullptr
                 *  in dst.node to flag that this is tip node.
                 *
                 *  Either:
                 *              X---- [curr_node] ----> [next_node]
                 *  Or:
                 *  [prev_node] <---- [curr_node] ----X
                 */

                FreeChain const src(curr_node, TerminusType::NONE, 0);
                FreeChain const dst(nullptr, TerminusType::NONE, 0);
                insert_points.emplace_back(src, dst);
            }

            if (next_node) {
                /*
                 * curr_node and next_node are linked.
                 * [curr_node] -------------link1-------------> [next_node]
                 *
                 * Find all ptlink1, ptlink2 that:
                 * [curr_node] -ptlink1-> [new_node] -ptlink2-> [next_node]
                 *
                 * Where src chain_id and term are known for curr_node, and
                 * dst chain_id and term are known for next_node.
                 */
                Link const* link1 = curr_node->find_link_to(next_node);

                insert_points.emplace_back(link1->src(), link1->dst());
                BridgePoint const& ip = insert_points.back();
                if (ip.bridges.size() == 0) {
                    insert_points.pop_back();
                }
            }
        } while (not node_gtor.is_done());

        // insert_points will at least contain the tip nodes.
        DEBUG(insert_points.empty());

        // Insert a node using a random insert point
        BridgePoint const& insert_point = insert_points.pick_random();
        if (insert_point.dst.node) {
            // This is a non-tip node.
            FreeChain const& port1 = insert_point.src;
            FreeChain const& port2 = insert_point.dst;
            Node* node1 = port1.node;
            Node* node2 = port2.node;

            // Break link
            node1->remove_link(port1);
            node2->remove_link(port2);

            // Pick a random bridge.
            auto const& bridge = insert_point.bridges.pick_random();

            // Create a new node in the middle.
            Node* new_node = new Node(
                bridge.ptlink1->module(),
                node1->tx_ * bridge.ptlink1->tx());
            nodes_.push_back(new_node);

            /*
             * Link up
             * Old link  ------------------link------------------->
             *           port1                                port2
             *
             * [ node1 ] <--new_link1-- [ new_node ] --new_link2--> [ node2 ]
             *              /       \                  /       \
             *          port1 --- nn_src1          nn_src2 --- port2
             *         --new_link1_rev-->
             *
             * Prototype ---ptlink1--->              ---ptlink2--->
             */
            FreeChain nn_src1(
                new_node, port2.term, bridge.ptlink1->chain_id());
            Link const new_link1_rev(port1, bridge.ptlink1, nn_src1);

            node1->add_link(new_link1_rev);
            new_node->add_link(new_link1_rev.reversed());

            FreeChain nn_src2(
                new_node, port1.term, bridge.ptlink2->reverse()->chain_id());
            Link const new_link2(nn_src2, bridge.ptlink1, port2);

            new_node->add_link(new_link2);
            node2->add_link(new_link2.reversed());

            fix_limb_transforms(new_link2);
        }
        else {
            // This is a tip node. Inserting is trivial - same as
            // regenerate(). There is guranteed to have one FreeChain.

            bool free_chain_found = false;
            for (FreeChain const& fc : free_chains_) {
                if (fc.node == insert_point.src.node) {
                    grow_tip(fc);
                    free_chain_found = true;
                    break;
                }
            }

            if (not free_chain_found) {
                err("FreeChain not found for %s\n",
                    insert_point.src.node->to_string().c_str());
                err("Available FreeChain(s):\n");
                for (FreeChain const& fc : free_chains_) {
                    err("%s\n", fc.to_string().c_str());
                }
                NICE_PANIC(not free_chain_found);
            }
        }

        mutate_success = true;
    }
#endif

    return mutate_success;
}

bool BasicNodeTeam::swap_mutate() {
    bool mutate_success = false;

#ifndef NO_SWAP
    if (not free_chains_.empty()) {
        // Walk through all links to collect swap points.
        Vector<BridgePoint> swap_points;

        // Starting at either end is fine.
        DEBUG(free_chains_.size() != 2);
        Node* start_node = free_chains_[0].node;
        BasicNodeGenerator node_gtor(start_node);

        Node* prev_node = nullptr, * curr_node = nullptr;
        Node* next_node = node_gtor.next(); // starts with start_node
        do {
            prev_node = curr_node;
            curr_node = next_node;
            next_node = node_gtor.next(); // can be nullptr
            size_t const num_links = curr_node->links().size();

            if (num_links == 1) {
                /*
                 *  curr_node is a tip node. A tip node can be swapped by
                 *  deleting it, then randomly growing the tip into a
                 *  different ProtoModule. The only time this is not possible
                 *  is when the neighbor ProtoModule has no other ProtoLinks
                 *  on the terminus in question.
                 *
                 *  Either:
                 *              X---- [curr_node] ----> [next_node]
                 *  Or:
                 *  [prev_node] <---- [curr_node] ----X
                 */

                // Check that neighbor can indead grow into a different
                // ProtoModule.
                FreeChain const& dst_fc = curr_node->links().at(0).dst();
                ProtoModule const* neighbor = dst_fc.node->prototype();
                ProtoChain const& chain = neighbor->chains().at(dst_fc.chain_id);

                if (chain.get_term(dst_fc.term).links().size() > 1) {
                    FreeChain const src(curr_node, TerminusType::NONE, 0);
                    FreeChain const dst(nullptr, TerminusType::NONE, 0);
                    swap_points.emplace_back(src, dst);
                }
            }

            if (prev_node and next_node) {
                /*
                 * Linkage:
                 * [prev_node] --link1--> [curr_node] --link2--> [next_node]
                 *             src                           dst
                 *
                 * Find all ptlink1, ptlink2 that:
                 * [prev_node] -ptlink1-> [diff_node] -ptlink2-> [next_node]
                 *
                 * Where src chain_id and term are known for curr_node, and
                 * dst chain_id and term are known for next_node.
                 */
                Link const* link1 = prev_node->find_link_to(curr_node);
                Link const* link2 = curr_node->find_link_to(next_node);

                swap_points.emplace_back(link1->src(), link2->dst());
                BridgePoint const& sp = swap_points.back();
                if (sp.bridges.size() == 0) {
                    swap_points.pop_back();
                }
            }
        } while (not node_gtor.is_done());

        // swap_points may not even contain tip nodes if they can't possibly
        // be swapped.
        if (not swap_points.empty()) {
            // Insert a node using a random insert point
            BridgePoint const& swap_point = swap_points.pick_random();
            if (swap_point.dst.node) {
                // This is a non-tip node.
                FreeChain const& port1 = swap_point.src;
                FreeChain const& port2 = swap_point.dst;
                Node* node1 = port1.node;
                Node* node2 = port2.node;

                // Break link
                node1->remove_link(port1);
                node2->remove_link(port2);

                // Pick a random bridge.
                auto const& bridge = swap_point.bridges.pick_random();

                // Create a new node in the middle.
                Node* new_node = new Node(
                    bridge.ptlink1->module(),
                    node1->tx_ * bridge.ptlink1->tx());
                nodes_.push_back(new_node);

                /*
                 * Link up
                 * Old link  ------------------link------------------->
                 *           port1                                port2
                 *
                 * [ node1 ] <--new_link1-- [ new_node ] --new_link2--> [ node2 ]
                 *              /       \                  /       \
                 *          port1 --- nn_src1          nn_src2 --- port2
                 *         --new_link1_rev-->
                 *
                 * Prototype ---ptlink1--->              ---ptlink2--->
                 */
                FreeChain nn_src1(
                    new_node, port2.term, bridge.ptlink1->chain_id());
                Link const new_link1_rev(port1, bridge.ptlink1, nn_src1);

                node1->add_link(new_link1_rev);
                new_node->add_link(new_link1_rev.reversed());

                FreeChain nn_src2(
                    new_node, port1.term, bridge.ptlink2->reverse()->chain_id());
                Link const new_link2(nn_src2, bridge.ptlink1, port2);

                new_node->add_link(new_link2);
                node2->add_link(new_link2.reversed());

                fix_limb_transforms(new_link2);
            }
            else {
                // This is a tip node. Inserting is trivial - same as
                // regenerate(). There is guranteed to have one FreeChain.

                bool free_chain_found = false;
                for (FreeChain const& fc : free_chains_) {
                    if (fc.node == swap_point.src.node) {
                        grow_tip(fc);
                        free_chain_found = true;
                        break;
                    }
                }

                if (not free_chain_found) {
                    err("FreeChain not found for %s\n",
                        swap_point.src.node->to_string().c_str());
                    err("Available FreeChain(s):\n");
                    for (FreeChain const& fc : free_chains_) {
                        err("%s\n", fc.to_string().c_str());
                    }
                    NICE_PANIC(not free_chain_found);
                }
            }

            mutate_success = true;
        }
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
    NodeTeam const& mother,
    NodeTeam const& father) {
    IdPairs crossing_ids;

    size_t const mn_len = mother.size();;
    size_t const fn_len = father.size();

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
    NodeTeam const* father) {
    bool mutate_success = false;

#ifndef NO_CROSS
    DEBUG(size() == 0);

    Nodes const& mother_nodes = nodes_; // Self has already inherited mother
    Nodes const& father_nodes = father->nodes_;
    IdPairs crossing_ids = get_crossing_ids(mother_nodes, father_nodes);

    size_t tries = 0;
    // cross can fail - if resultant nodes collide during synth
    while (tries < MAX_FREECANDIDATE_MUTATE_FAILS and
            crossing_ids.size()) {
        // Pick random crossing point
        IdPair const& cross_point = pick_random(crossing_ids);

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
    bool mutate_success = false;

#ifndef NO_REGENERATE
    if (nodes_.empty()) {
        // Pick random initial member
        add_member(XDB.basic_mods().draw());
    }

    FreeChain free_chain_a = free_chains_.pick_random();
    while (size() < Candidate::MAX_LEN) {
        grow_tip(free_chain_a);

        // Pick next tip chain
        free_chain_a = free_chains_.pick_random();
    }

    mutate_success = true;
#endif  /* ifndef NO_REGENERATE */

    return mutate_success;
}

bool BasicNodeTeam::randomize_mutate() {
    disperse();
    bool const mutate_success = regenerate();
    return mutate_success;
}


/* public */
/* ctors */
BasicNodeTeam::BasicNodeTeam(BasicNodeTeam const& other) {
    deep_copy_from(&other);
}

BasicNodeTeam* BasicNodeTeam::clone() const {
    return new BasicNodeTeam(*this);
}

/* modifiers */
void BasicNodeTeam::deep_copy_from(
    NodeTeam const* other) {
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
            node_ptr->update_link_ptrs(addr_map);
            nodes_.push_back(node_ptr);
        }

        for (auto& fc : free_chains_) {
            fc.node = addr_map.at(fc.node);
        }

        checksum_ = other->checksum();
        score_ = other->score();
    }
}

MutationMode BasicNodeTeam::mutate_and_score(
    NodeTeam const* mother,
    NodeTeam const* father,
    WorkArea const* wa) {
    // Inherit from mother
    deep_copy_from(mother);

    MutationModeList modes = gen_mutation_mode_list();

    bool mutate_success = false;
    MutationMode mode;

    while (not mutate_success and not modes.empty()) {
        mode = modes.pop_random();
        switch (mode) {
        case MutationMode::ERODE:
            mutate_success = erode_mutate();
            break;
        case MutationMode::DELETE:
            mutate_success = delete_mutate();
            break;
        case MutationMode::INSERT:
            mutate_success = insert_mutate();
            break;
        case MutationMode::SWAP:
            mutate_success = swap_mutate();
            break;
        case MutationMode::CROSS:
            mutate_success = cross_mutate(father);
            break;
        case MutationMode::REGENERATE:
            mutate_success = regenerate();
            break;
        default:
            bad_mutation_mode(mode);
        }
    }

    if (not mutate_success) {
        mutate_success = randomize_mutate();
        NICE_PANIC(not mutate_success,
                   "Randomize Mutate also failed - bug?");
    }

    // Update checksum
    checksum_ = calc_checksum();

    // Update score
    score_ = calc_score(wa);

    return mode;
}

/* printers */
std::string BasicNodeTeam::to_string() const {
    std::ostringstream ss;

    NICE_PANIC(free_chains_.empty());
    Node* start_node = free_chains_.at(0).node;
    BasicNodeGenerator node_gtor(start_node);

    while (not node_gtor.is_done()) {
        ss << node_gtor.next()->to_string();
        Link const* link_ptr = node_gtor.curr_link();
        if (link_ptr) {
            TerminusType const src_term = link_ptr->src().term;
            TerminusType const dst_term = link_ptr->dst().term;
            ss << "\n(" << TerminusTypeToCStr(src_term) << ", ";
            ss << TerminusTypeToCStr(dst_term) << ")\n";
        }
    }

    return ss.str();
}

StrList BasicNodeTeam::get_node_names() const {
    StrList res;

    if (not free_chains_.empty()) {
        NICE_PANIC(free_chains_.size() != 2);

        // Start at either tip
        BasicNodeGenerator node_gtor(free_chains_.at(0).node);

        while (not node_gtor.is_done()) {
            res.emplace_back(node_gtor.next()->prototype()->name);
        }

        DEBUG(res.size() != size(),
              string_format("res.size()=%lu, size()=%lu\n",
                            res.size(), this->size()));
    }
    else {
        res.emplace_back("[Empty]");
    }

    return res;
}

}  /* elfin */