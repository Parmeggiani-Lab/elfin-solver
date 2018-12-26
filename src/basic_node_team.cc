#include "basic_node_team.h"

#include <jutil/jutil.h>
#include "basic_node_generator.h"
#include "kabsch.h"
#include "input_manager.h"
#include "id_types.h"
#include "pointer_utils.h"

// #define NO_ERODE
// #define NO_DELETE
// #define NO_INSERT
// #define NO_SWAP
// #define NO_CROSS

#if defined(NO_ERODE) || \
defined(NO_DELETE) || \
defined(NO_INSERT) || \
defined(NO_SWAP) || \
defined(NO_CROSS)
#warning "At least one mutation function is DISABLED!!"
#endif

namespace elfin {

/* private */

struct BasicNodeTeam::PImpl {
    /* data */
    BasicNodeTeam* that;

    // If there is any other data member then PImpl needs to implement
    // copiers.

    /* ctors */
    PImpl(BasicNodeTeam* const _that) : that(_that) {}

    /* types */
    struct DeletePoint {
        //
        // [neighbor1] <--link1-- [delete_node] --link2--> [neighbor2]
        //             dst----src               src----dst
        //             ^^^                             ^^^
        //            (src)                           (dst)
        //             --------------skipper------------->
        //
        NodeSP delete_node;
        FreeChain const src, dst;
        ProtoLink const* skipper;
        DeletePoint(
            NodeSP const& _delete_node,
            FreeChain const&  _src,
            FreeChain const&  _dst,
            ProtoLink const* _skipper) :
            src(_src),
            dst(_dst),
            delete_node(_delete_node),
            skipper(_skipper) {}
    };

    struct InsertPoint {
        //
        // [  node1 ] --------------link-------------> [ node2  ]
        //            src                          dst
        //
        // Each bridge has pt_link1 and pt_link2 that:
        // [  node1 ] -pt_link1-> [new_node] -pt_link2-> [ node2  ]
        //
        FreeChain const src, dst;
        FreeChain::BridgeList const bridges;
        InsertPoint(
            FreeChain const& _src,
            FreeChain const& _dst) :
            src(_src),
            dst(_dst),
            bridges(is_uninitialized(dst.node) ?
                    FreeChain::BridgeList() :
                    src.find_bridges(dst)) { }
    };

    struct SwapPoint : public InsertPoint {
        NodeSP const del_node;
        SwapPoint(
            FreeChain const& _src,
            NodeSP const& _del_node,
            FreeChain const& _dst) :
            InsertPoint(_src, _dst),
            del_node(_del_node) {}
    };

    struct CrossPoint {
        ProtoLink const* pt_link;
        Link const* m_arrow;
        bool m_rev;
        Link const* f_arrow;
        bool f_rev;
        CrossPoint(
            ProtoLink const* _pt_link,
            Link const* _m_arrow,
            bool const _m_rev,
            Link const* _f_arrow,
            bool const _f_rev) :
            pt_link(_pt_link),
            m_arrow(_m_arrow),
            m_rev(_m_rev),
            f_arrow(_f_arrow),
            f_rev(_f_rev) {}
    };

    /* accessors */
    Crc32 calc_checksum() const {
        //
        // We want the same checksum for two node teams that consist of the same
        // sequence of nodes even if they are in reverse order. This can be
        // achieved by XOR'ing the forward and backward checksums.
        //
        DEBUG(that->free_chains_.size() != 2,
              string_format("There are %zu free chains!",
                            that->free_chains_.size()));
        DEBUG(that->size() == 0);

        Crc32 crc = 0x0000;
        for (auto& free_chain : that->free_chains_) {
            BasicNodeGenerator node_gen(free_chain.node_sp());

            Crc32 crc_half = 0xffff;
            while (not node_gen.is_done()) {
                NodeSP node = node_gen.next();
                ProtoModule const* prot = node->prototype_;
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

    float calc_score() const {
        //
        // We score the path forward and backward, because the Kabsch
        // algorithm relies on point-wise correspondance. Different ordering
        // can yield different RMSD scores.
        //
        DEBUG(that->free_chains_.size() != 2,
              string_format("There are %zu free chains!\n",
                            that->free_chains_.size()));
        DEBUG(that->size() == 0);

        float score = INFINITY;
        for (auto& free_chain : that->free_chains_) {
            V3fList points = collect_points(free_chain.node_sp());

            float const new_score =
                kabsch::score(points, that->work_area_->points());
            score = new_score < score ? new_score : score;
        }

        return score;
    }

    V3fList collect_points(NodeSP tip_node) const {
        // Verify node is a tip node.
        NICE_PANIC(tip_node->links().size() > 1);

        BasicNodeGenerator node_gen(tip_node);

        V3fList points;
        while (not node_gen.is_done()) {
            points.push_back(node_gen.next()->tx_.collapsed());
        }

        DEBUG(points.size() != that->size(),
              string_format("points.size()=%zu, size()=%zu\n",
                            points.size(), that->size()));

        return points;
    }

    /*modifiers */
    void fix_limb_transforms(Link const& arrow) {
        BasicNodeGenerator limb_gen(&arrow);
        while (not limb_gen.is_done()) {
            Link const* curr_link = limb_gen.curr_link();
            NodeSP curr_node = limb_gen.curr_node();
            NodeSP next_node = limb_gen.next();
            next_node->tx_ = curr_node->tx_ * curr_link->prototype()->tx_;
        }
    }

    NodeSP grow_tip(
        FreeChain const free_chain_a,
        ProtoLink const* pt_link = nullptr) {
        if (pt_link == nullptr) {
            pt_link = &free_chain_a.random_proto_link();
        }

        NodeSP node_a = free_chain_a.node_sp();

        // Verify that provided free chain is attached a node that is a tip
        // node.
        DEBUG(node_a->links().size() > 1);

        auto node_b = that->add_member(
                          pt_link->module_,
                          node_a->tx_ * pt_link->tx_);

        TerminusType const term_a = free_chain_a.term;
        TerminusType const term_b = opposite_term(term_a);

        FreeChain const free_chain_b =
            FreeChain(node_b, term_b, pt_link->chain_id_);

        node_a->add_link(free_chain_a, pt_link, free_chain_b);
        node_b->add_link(free_chain_b, pt_link->reverse(), free_chain_a);

        that->free_chains_.lift_erase(free_chain_a);
        that->free_chains_.lift_erase(free_chain_b);

        return node_b;
    }

    //Removes the selected tip.
    void nip_tip(
        NodeSP tip_node) {
        // Verify node is a tip node.
        size_t const num_links = tip_node->links().size();
        NICE_PANIC(num_links != 1);

        NodeSP new_tip = nullptr;

        FreeChain const& new_free_chain =
            tip_node->links().at(0).dst();
        new_tip = new_free_chain.node_sp();

        // Unlink chain.
        new_tip->remove_link(new_free_chain);

        // Restore FreeChain.
        that->free_chains_.push_back(new_free_chain);


        that->remove_free_chains(tip_node);
        that->nodes_.erase(tip_node);
    }

    void build_bridge(
        InsertPoint const& insert_point,
        FreeChain::Bridge const* bridge = nullptr) {
        if (bridge == nullptr) {
            bridge = &insert_point.bridges.pick_random();
        }

        FreeChain const& port1 = insert_point.src;
        FreeChain const& port2 = insert_point.dst;
        NodeSP node1 = port1.node_sp();
        NodeSP node2 = port2.node_sp();

        // Break link.
        node1->remove_link(port1);
        node2->remove_link(port2);

        // Create a new node in the middle.
        auto new_node = std::make_shared<Node>(
                            bridge->pt_link1->module_,
                            node1->tx_ * bridge->pt_link1->tx_);
        that->nodes_.push_back(new_node);

        //
        // Link up
        // Old link  ------------------link------------------->
        //           port1                                port2
        //
        // [ node1 ] <--new_link1-- [ new_node ] --new_link2--> [ node2 ]
        //              /       \                  /       \
        //          port1 --- nn_src1          nn_src2 --- port2
        //         --new_link1_rev-->
        //
        // Prototype ---pt_link1--->              ---pt_link2--->
        //
        FreeChain nn_src1(
            new_node, port2.term, bridge->pt_link1->chain_id_);
        Link const new_link1_rev(port1, bridge->pt_link1, nn_src1);

        node1->add_link(new_link1_rev);
        new_node->add_link(new_link1_rev.reversed());

        FreeChain nn_src2(
            new_node, port1.term, bridge->pt_link2->reverse()->chain_id_);
        Link const new_link2(nn_src2, bridge->pt_link2, port2);

        new_node->add_link(new_link2);
        node2->add_link(new_link2.reversed());

        fix_limb_transforms(new_link2);
    }

    void sever_limb(Link const& arrow) {
        // Delete the dst side of arrow.
        DEBUG(is_uninitialized(arrow.src().node));
        DEBUG(is_uninitialized(arrow.dst().node));

        Link curr_arrow = arrow;
        size_t num_links = 0;
        do {
            // Unlink nodes.
            NodeSP next_node = curr_arrow.dst().node_sp();
            next_node->remove_link(curr_arrow.dst());

            // Verify temporary tip node.
            num_links = next_node->links().size();
            DEBUG(num_links > 1); // Must be 0 or 1.

            if (num_links > 0) {
                curr_arrow = next_node->links().at(0); // Copy.
            }
            else {
                that->remove_free_chains(next_node);
            }

            // Safe to delete node.
            that->nodes_.erase(next_node);
        } while (num_links > 0);

        arrow.src().node_sp()->remove_link(arrow.src());
        that->free_chains_.push_back(arrow.src());
    }

    // Copy nodes starting from f_arrow.dst() to m_arrow.src().
    void copy_limb(
        Link const& m_arrow,
        Link const& f_arrow) {
        DEBUG(that->size() == 0);

        NodeSP tip_node = m_arrow.src().node_sp();
        size_t num_links = tip_node->links().size();
        DEBUG(that->size() > 1 and num_links != 1);

        // Occupy src chain.
        that->free_chains_.lift_erase(m_arrow.src());

        // Form first link.
        ProtoLink const* pt_link =
            tip_node->prototype_->find_link_to(
                m_arrow.src().chain_id,
                m_arrow.src().term,
                f_arrow.dst().node_sp()->prototype_,
                f_arrow.dst().chain_id);
        DEBUG(pt_link == nullptr);

        tip_node = grow_tip(m_arrow.src(), pt_link);

        num_links = tip_node->links().size();
        DEBUG(num_links != 1,
              string_format("There are %zu links!\n", num_links));

        BasicNodeGenerator node_gen(&f_arrow);
        node_gen.next(); // Same as f_arrow.dst().node.
        while (not node_gen.is_done()) {
            Link const* curr_link = node_gen.curr_link();
            DEBUG(curr_link->dst().node_sp()->prototype_ !=
                  curr_link->prototype()->module_);

            // Modify copy of curr_link->src().
            FreeChain src = curr_link->src();

            DEBUG(src.node_sp()->prototype_ != tip_node->prototype_,
                  string_format("%s vs %s\n",
                                src.node_sp()->prototype_->name.c_str(),
                                tip_node->prototype_->name.c_str()));
            src.node = tip_node;

            // Occupy src chain.
            that->free_chains_.lift_erase(src);
            DEBUG(that->free_chains_.size() != 1,
                  string_format("There are %zu free chains!",
                                that->free_chains_.size()));

            tip_node = grow_tip(src, curr_link->prototype());
            if (curr_link->dst().node_sp()->prototype_ != tip_node->prototype_) {
                err("%s vs %s\n",
                    curr_link->dst().node_sp()->prototype_->name.c_str(),
                    tip_node->prototype_->name.c_str());
                err("curr_link->prototype(): %s\n",
                    curr_link->prototype()->module_->name.c_str());
                die("");
            }

            num_links = tip_node->links().size();
            DEBUG(num_links != 1,
                  string_format("There are %zu links!\n", num_links));

            node_gen.next();
        }
    }

    /* mutation methods */
    bool erode_mutate() {

        bool mutate_success = false;
#ifndef NO_ERODE
        if (not that->free_chains_.empty()) {
            // Pick random tip node if not specified.
            NodeSP tip_node = that->free_chains_.pick_random().node_sp();
            that->remove_free_chains(tip_node);

            FreeChain last_free_chain;
            float p = 1.0f; // p for Probability.

            // Loop condition is always true on first entrance, hence do-while.
            bool next_loop = false;
            do {
                // Calculate next iteration condition.
                //
                // The following formula gives:
                // remaining size / original size; p
                // 6/6; p=0.8333333333333334
                // 5/6; p=0.6666666666666667
                // 4/6; p=0.5
                // 3/6; p=0.3333333333333333
                // 2/6; p=0.16666666666666666
                // 1/6; p=0.0
                p = p * (that->size() - 2) / (that->size() - 1); // size() is at least 2
                next_loop = random::get_dice_0to1() <= p;

                // Verify node is tip node.
                size_t const num_links = tip_node->links().size();
                NICE_PANIC(num_links != 1);

                FreeChain const& new_free_chain =
                    tip_node->links().at(0).dst();
                NodeSP new_tip = new_free_chain.node_sp();

                // Unlink
                new_tip->remove_link(new_free_chain);

                if (not next_loop) {
                    last_free_chain = new_free_chain;
                }

                that->nodes_.erase(tip_node);

                tip_node = new_tip;
            } while (next_loop);

            // Restore chain.
            that->free_chains_.push_back(last_free_chain);

            regenerate();

            mutate_success = true;
        }
#endif // NO_ERODE

        return mutate_success;
    }

    bool delete_mutate() {

        bool mutate_success = false;
#ifndef NO_DELETE
        if (not that->free_chains_.empty()) {
            // Walk through all nodes to collect delete points.
            Vector<DeletePoint> delete_points;

            // Starting at either end is fine.
            NodeSP start_node = that->free_chains_[0].node_sp();
            BasicNodeGenerator node_gen(start_node);

            NodeSP curr_node = nullptr;
            NodeSP next_node = node_gen.next(); // Starts with start_node.
            do {
                curr_node = next_node;
                next_node = node_gen.next(); // Can be nullptr.
                size_t const num_links = curr_node->links().size();

                if (num_links == 1) {
                    //
                    // curr_node is a tip node, which can always be deleted trivially.
                    // Use ProtoLink* = nullptr to mark a tip node. Pointers
                    // src and dst are not used.
                    //

                    delete_points.emplace_back(
                        /*delete_node=*/ curr_node,
                        /*src=*/ FreeChain(),
                        /*dst=*/ FreeChain(),
                        /*skipper=*/ nullptr);
                }
                else if (num_links == 2) {
                    //
                    // curr_node is between start and end node. Find a link that
                    // skips curr_node. The reverse doesn't need to be checked,
                    // because all links have a reverse.
                    //

                    Link const& link1 = curr_node->links().at(0);
                    Link const& link2 = curr_node->links().at(1);
                    FreeChain const& src = link1.dst();
                    FreeChain const& dst = link2.dst();

                    //
                    // X--[neighbor1]--src->-<-dst               dst->-<-src--[neighbor2]--...
                    //                 (  link1  )               (  link2  )
                    //                 vvvvvvvvvvv               vvvvvvvvvvv
                    //                 dst->-<-src--[curr_node]--src->-<-dst
                    //                 ^^^                               ^^^
                    //                (src)                             (dst)
                    //
                    ProtoLink const* const proto_link_ptr =
                        src.find_link_to(dst);
                    if (proto_link_ptr) {
                        delete_points.emplace_back(
                            /*delete_node=*/ curr_node,
                            /*src=*/ src,
                            /*dst=*/ dst,
                            /*skipper=*/ proto_link_ptr);
                    }
                }
                else {
                    NICE_PANIC("Unexpected num_links",
                               string_format(
                                   "Number of links: %zu\n", num_links));
                }
            } while (not node_gen.is_done());

            // delete_points will at least contain the tip nodes.
            DEBUG(delete_points.empty());

            // Delete a node using a random deletable point.
            DeletePoint const& delete_point = delete_points.pick_random();
            if (delete_point.skipper) {
                //
                // This is NOT a tip node. Need to do some clean up
                //
                // Link up neighbor1 and neighbor2
                // X--[neighbor1]--src->-<-dst                 dst->-<-src--[neighbor2]--...
                //                 (  link1  )                 (  link2  )
                //                 vvvvvvvvvvv                 vvvvvvvvvvv
                //                 dst->-<-src--[delete_node]--src->-<-dst
                //                 ^^^                                 ^^^
                //                (src)                               (dst)
                //
                NodeSP neighbor1 = delete_point.src.node_sp();
                NodeSP neighbor2 = delete_point.dst.node_sp();

                neighbor1->remove_link(delete_point.src);
                neighbor2->remove_link(delete_point.dst);
                //
                // X--[neighbor1]--X                                     X--[neighbor2]--...
                //                 (  link1  )                 (  link2  )
                //                 vvvvvvvvvvv                 vvvvvvvvvvv
                //                 dst->-<-src--[delete_node]--src->-<-dst
                //                 ----------------arrow1---------------->
                //

                // Create links between neighbor1 and neighbor2.
                Link const arrow1(delete_point.src,
                                  delete_point.skipper,
                                  delete_point.dst);
                neighbor1->add_link(arrow1);
                neighbor2->add_link(arrow1.reversed());
                //
                //         link1->dst()     link2->dst()
                //                  vvv     vvv
                //  X--[neighbor1]--src->-<-dst
                //                  dst->-<-src--[neighbor2]--...
                //                  ^^^     ^^^
                //         link1->dst()     link2->dst
                //

                that->nodes_.erase(delete_point.delete_node);

                // delete_node is guranteed to not be a tip so no need to clean up
                // that->free_chains_.
                fix_limb_transforms(arrow1);
            }
            else {
                // This is a tip node.
                nip_tip(delete_point.delete_node);
            }

            mutate_success = true;
        }
#endif
        return mutate_success;
    }

    bool insert_mutate() {

        bool mutate_success = false;
#ifndef NO_INSERT
        if (not that->free_chains_.empty()) {
            // Walk through all links to collect insert points.
            Vector<InsertPoint> insert_points;

            // Starting at either end is fine.
            NodeSP start_node = that->free_chains_[0].node_sp();
            BasicNodeGenerator node_gen(start_node);

            NodeSP curr_node = nullptr;
            NodeSP next_node = node_gen.next(); // Starts with start_node.
            do {
                curr_node = next_node;
                next_node = node_gen.next(); // Can be nullptr.
                size_t const num_links = curr_node->links().size();

                if (num_links == 1) {
                    //
                    // curr_node is a tip node. A new node can be inserted on the
                    // unconnected terminus of a tip node trivially. Use nullptr
                    // in dst.node to flag that this is tip node.
                    //
                    // Either:
                    //             X---- [curr_node] ----> [next_node]
                    // Or:
                    // [prev_node] <---- [curr_node] ----X
                    //

                    FreeChain const src(curr_node, TerminusType::NONE, 0);
                    insert_points.emplace_back(src, FreeChain());
                }

                if (next_node) {
                    //
                    // curr_node and next_node are linked.
                    // [curr_node] -------------link1-------------> [next_node]
                    //
                    // Find all pt_link1, pt_link2 that:
                    // [curr_node] -pt_link1-> [new_node] -pt_link2-> [next_node]
                    //
                    // Where src chain_id and term are known for curr_node, and
                    // dst chain_id and term are known for next_node.
                    //
                    Link const* link1 = curr_node->find_link_to(next_node);

                    insert_points.emplace_back(link1->src(), link1->dst());
                    InsertPoint const& ip = insert_points.back();
                    if (ip.bridges.size() == 0) {
                        insert_points.pop_back();
                    }
                }
            } while (not node_gen.is_done());

            // insert_points will at least contain the tip nodes.
            DEBUG(insert_points.empty());

            // Insert a node using a random insert point/
            InsertPoint const& insert_point = insert_points.pick_random();
            if (not is_uninitialized(insert_point.dst.node)) {
                // This is a non-tip node.
                build_bridge(insert_point);
            }
            else {
                // This is a tip node. Inserting is the same as grow_tip().
                bool free_chain_found = false;
                for (FreeChain const& fc : that->free_chains_) {
                    if (fc.node_sp() == insert_point.src.node_sp()) {
                        grow_tip(fc);
                        free_chain_found = true;
                        break;
                    }
                }

                if (not free_chain_found) {
                    err("FreeChain not found for %s\n",
                        insert_point.src.node_sp()->to_string().c_str());
                    err("Available FreeChain(s):\n");
                    for (FreeChain const& fc : that->free_chains_) {
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

    bool swap_mutate() {

        bool mutate_success = false;
#ifndef NO_SWAP
        if (not that->free_chains_.empty()) {
            // Walk through all links to collect swap points.
            Vector<SwapPoint> swap_points;

            // Starting at either end is fine.
            NodeSP start_node = that->free_chains_[0].node_sp();
            BasicNodeGenerator node_gen(start_node);

            NodeSP prev_node = nullptr;
            NodeSP curr_node = nullptr;
            NodeSP next_node = node_gen.next(); // Starts with start_node/
            do {
                prev_node = curr_node;
                curr_node = next_node;
                next_node = node_gen.next(); // Can be nullptr.
                size_t const num_links = curr_node->links().size();

                if (num_links == 1) {
                    //
                    // curr_node is a tip node. A tip node can be swapped by
                    // deleting it, then randomly growing the tip into a
                    // different ProtoModule. The only time this is not possible
                    // is when the neighbor ProtoModule has no other ProtoLinks
                    // on the terminus in question.
                    //
                    //             X---- [curr_node] ----> [some_node]
                    //

                    // Check that neighbor can indead grow into a different
                    // ProtoModule.
                    FreeChain const& tip_fc = curr_node->links().at(0).dst();
                    ProtoModule const* neighbor = tip_fc.node_sp()->prototype_;
                    ProtoChain const& chain = neighbor->chains().at(tip_fc.chain_id);

                    if (chain.get_term(tip_fc.term).links().size() > 1) {
                        swap_points.emplace_back(tip_fc, curr_node, FreeChain());
                    }
                }

                if (prev_node and next_node) {
                    //
                    // Linkage:
                    // [prev_node] --link1--> [curr_node] --link2--> [next_node]
                    //             src                           dst
                    //
                    // Find all pt_link1, pt_link2 that:
                    // [prev_node] -pt_link1-> [diff_node] -pt_link2-> [next_node]
                    //
                    // Where src chain_id and term are known for curr_node, and
                    // dst chain_id and term are known for next_node.
                    //
                    Link const* link1 = prev_node->find_link_to(curr_node);
                    Link const* link2 = curr_node->find_link_to(next_node);

                    swap_points.emplace_back(link1->src(), curr_node, link2->dst());
                    SwapPoint const& sp = swap_points.back();
                    if (sp.bridges.size() == 0) {
                        swap_points.pop_back();
                    }
                }
            } while (not node_gen.is_done());

            // swap_points may not even contain tip nodes if they can't possibly
            // be swapped.
            if (not swap_points.empty()) {
                // Insert a node using a random insert point.
                SwapPoint const& swap_point = swap_points.pick_random();
                if (not is_uninitialized(swap_point.dst.node)) {
                    // This is a non-tip node.
                    that->nodes_.erase(swap_point.del_node);
                    build_bridge(swap_point);
                }
                else {
                    // This is a tip node.
                    nip_tip(swap_point.del_node);
                    grow_tip(swap_point.src);
                }

                mutate_success = true;
            }
        }
#endif
        return mutate_success;
    }

    bool cross_mutate(NodeTeam const& father) {

        bool mutate_success = false;
#ifndef NO_CROSS
        if (not that->free_chains_.empty() and
                not father.free_chains().empty()) {
            // First, collect arrows from both parents.

            // Starting at either end is fine.
            auto m_arrows = BasicNodeGenerator::collect_arrows(
                                that->free_chains_[0].node_sp());
            DEBUG(father.free_chains().size() != 2);
            auto f_arrows = BasicNodeGenerator::collect_arrows(
                                father.free_chains()[0].node_sp());

            // Walk through all link pairs to collect cross points.
            Vector<CrossPoint> cross_points;
            for (Link const* m_arrow : m_arrows) {
                for (Link const* f_arrow : f_arrows) {
                    ProtoLink const* sd =
                        m_arrow->src().find_link_to(f_arrow->dst());
                    if (sd) {
                        cross_points.emplace_back(
                            sd,
                            m_arrow, false,   // { } --m_arrow--v { del  }
                            f_arrow, false);  // { } --f_arrow--> { copy }
                    }

                    ProtoLink const* ss =
                        m_arrow->src().find_link_to(f_arrow->src());
                    if (ss) {
                        cross_points.emplace_back(
                            ss,
                            m_arrow, false,   // {      } --m_arrow--v { del }
                            f_arrow, true);   // { copy } <---f_rev--- {     }
                    }

                    ProtoLink const* dd =
                        m_arrow->dst().find_link_to(f_arrow->dst());
                    if (dd) {
                        cross_points.emplace_back(
                            dd,
                            m_arrow, true,    // { del } v---m_rev--- {      }
                            f_arrow, false);  // {     } --f_arrow--> { copy }
                    }

                    ProtoLink const* ds =
                        m_arrow->dst().find_link_to(f_arrow->src());
                    if (ds) {
                        cross_points.emplace_back(
                            ds,
                            m_arrow, true,    // { del  } v---m_rev--- { }
                            f_arrow, true);   // { keep } <---f_rev--- { }
                    }
                }
            }

            if (not cross_points.empty()) {
                CrossPoint const& cp = cross_points.pick_random();

                Link m_arrow = cp.m_rev ?
                               cp.m_arrow->reversed() :
                               *cp.m_arrow; // Make a copy.
                Link f_arrow = cp.f_rev ?
                               cp.f_arrow->reversed() :
                               *cp.f_arrow; // Make a copy.

                // Always keep m_arrow.src, del m_arrow.dst, and copy from
                // f_arrow.dst.
                sever_limb(m_arrow);
                copy_limb(m_arrow, f_arrow);

                mutate_success = true;
            }
        }
#endif
        return mutate_success;
    }

    bool regenerate() {
        if (that->nodes_.empty()) {
            // Pick random initial member.
            that->add_member(XDB.basic_mods().draw());
        }

        FreeChain free_chain_a = that->free_chains_.pick_random();
        while (that->size() < that->work_area_->target_size()) {
            grow_tip(free_chain_a);

            // Pick next tip chain.
            free_chain_a = that->free_chains_.pick_random();
        }

        return true;
    }

    bool randomize_mutate() {
        that->nodes_.clear();
        that->free_chains_.clear();
        that->checksum_ = 0x0000;
        that->score_ = INFINITY;
        bool const mutate_success = regenerate();

        NICE_PANIC(that->free_chains_.size() != 2); // Replace with mutation_exit_check()

        return mutate_success;
    }
};

/* accessors */
Crc32 BasicNodeTeam::calc_checksum() const {
    return p_impl_->calc_checksum();
}

float BasicNodeTeam::calc_score() const {
    return p_impl_->calc_score();
}

V3fList BasicNodeTeam::collect_points(NodeSP const& tip_node) const {
    return p_impl_->collect_points(tip_node);
}

/* modifiers */
std::unique_ptr<BasicNodeTeam::PImpl> BasicNodeTeam::init_pimpl() {
    return std::make_unique<PImpl>(this);
}

NodeSP BasicNodeTeam::grow_tip(
    FreeChain const free_chain_a,
    ProtoLink const* pt_link) {
    return p_impl_->grow_tip(free_chain_a, pt_link);
}

/* protected */
/* accessors */
BasicNodeTeam * BasicNodeTeam::clone_impl() const {
    return new BasicNodeTeam(*this);
}

/* public */
/* ctors */
BasicNodeTeam::BasicNodeTeam(WorkArea const* wa) :
    NodeTeam(wa), p_impl_(init_pimpl()) {}

BasicNodeTeam::BasicNodeTeam(BasicNodeTeam const& other) :
    NodeTeam(other), p_impl_(init_pimpl()) {}

BasicNodeTeam::BasicNodeTeam(BasicNodeTeam&& other) :
    NodeTeam(std::move(other)), p_impl_(init_pimpl()) {}

/* dtors */
BasicNodeTeam::~BasicNodeTeam() {}

/* modifiers */
mutation::Mode BasicNodeTeam::mutate_and_score(
    NodeTeam const& mother,
    NodeTeam const& father) {
    // Inherit from mother.
    NodeTeam::operator=(mother);

    auto modes = mutation::gen_mode_list();

    bool mutate_success = false;
    mutation::Mode mode;

    while (not mutate_success and not modes.empty()) {
        NICE_PANIC(size() == 0);
        NICE_PANIC(free_chains_.size() != 2); // Replace with mutation_entry_check()

        mode = modes.pop_random();
        switch (mode) {
        case mutation::Mode::ERODE:
            mutate_success = p_impl_->erode_mutate();
            break;
        case mutation::Mode::DELETE:
            mutate_success = p_impl_->delete_mutate();
            break;
        case mutation::Mode::INSERT:
            mutate_success = p_impl_->insert_mutate();
            break;
        case mutation::Mode::SWAP:
            mutate_success = p_impl_->swap_mutate();
            break;
        case mutation::Mode::CROSS:
            mutate_success = p_impl_->cross_mutate(father);
            break;
        case mutation::Mode::REGENERATE:
            mutate_success = p_impl_->regenerate();
            break;
        default:
            mutation::bad_mode(mode);
        }

        NICE_PANIC(free_chains_.size() != 2); // Replace with mutation_exit_check()
    }

    if (not mutate_success) {
        mutate_success = p_impl_->randomize_mutate();
        NICE_PANIC(not mutate_success,
                   "Randomize Mutate also failed - bug?");
    }

    // Update checksum.
    checksum_ = p_impl_->calc_checksum();

    // Update score.
    score_ = p_impl_->calc_score();

    return mode;
}

void BasicNodeTeam::randomize() {
    p_impl_->randomize_mutate();
}

/* printers */
std::string BasicNodeTeam::to_string() const {
    std::ostringstream ss;

    NICE_PANIC(free_chains_.empty());
    NodeSP start_node = free_chains_.at(0).node_sp();
    BasicNodeGenerator node_gen(start_node);

    while (not node_gen.is_done()) {
        ss << node_gen.next()->to_string();
        Link const* link_ptr = node_gen.curr_link();
        if (link_ptr) {
            TerminusType const src_term = link_ptr->src().term;
            TerminusType const dst_term = link_ptr->dst().term;
            ss << "\n(" << TerminusTypeToCStr(src_term) << ", ";
            ss << TerminusTypeToCStr(dst_term) << ")\n";
        }
    }

    return ss.str();
}

JSON BasicNodeTeam::gen_nodes_json() const {
    JSON output;

    if (not free_chains_.empty()) {
        DEBUG(free_chains_.size() != 2);

        // Kabsch outputs for forward and backward paths.
        elfin::Mat3f rot[2];
        Vector3f tran[2];
        float rms[2];

        for (size_t i = 0; i < 2; ++i) {
            V3fList const& points = collect_points(free_chains_.at(i).node_sp());

            kabsch::calc_alignment(
                /* mobile */ points,
                /* ref */ work_area_->points(),
                rot[i],
                tran[i],
                rms[i]);
        }

        // Start at tip that yields lower score.
        size_t const better_tip_id = rms[0] < rms[1] ? 0 : 1;

        NodeSP tip_node = free_chains_.at(better_tip_id).node_sp();
        Transform kabsch_alignment(rot[better_tip_id], tran[better_tip_id]);

        size_t member_id = 0;  // UID for node in team.
        BasicNodeGenerator node_gen(tip_node);
        while (not node_gen.is_done()) {
            NodeSP curr_node = node_gen.next();

            JSON node_output;
            node_output["name"] = curr_node->prototype_->name;
            node_output["member_id"] = member_id;

            auto link = node_gen.curr_link();
            if (link) {  //  Not reached end of nodes yet.
                node_output["src_term"] =
                    TerminusTypeToCStr(link->src().term);

                std::string const& src_chain_name =
                    curr_node->prototype_->chains().at(
                        link->src().chain_id).name;
                std::string const& dst_chain_name =
                    node_gen.peek()->prototype_->chains().at(
                        link->dst().chain_id).name;
                node_output["src_chain_name"] = src_chain_name;
                node_output["dst_chain_name"] = dst_chain_name;
            }
            else
            {
                node_output["src_term"] =
                    TerminusTypeToCStr(TerminusType::NONE);
                node_output["src_chain_name"] = "NONE";
                node_output["dst_chain_name"] = "NONE";
            }

            Transform tx = curr_node->tx_;

            // Apply Kabsch alignment
            tx = kabsch_alignment * tx;

            node_output["rot"] = tx.rot_json();
            node_output["tran"] = tx.tran_json();

            member_id++;
            output.emplace_back(node_output);
        }

        DEBUG(output.size() != size(),
              string_format("output.size()=%zu, size()=%zu\n",
                            output.size(), this->size()));
    }

    return output;
}

}  /* elfin */