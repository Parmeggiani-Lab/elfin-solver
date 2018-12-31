#include "path_team.h"

#include "kabsch.h"
#include "path_generator.h"
#include "input_manager.h"
#include "id_types.h"
#include "mutation.h"

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
struct PathTeam::PImpl {
    /* data */
    PathTeam& _;

    /* ctors */
    PImpl(PathTeam& interface) : _(interface) {}

    /* accessors */
    V3fList collect_points(NodeKey tip_node) const {
        // Check node is a tip node.
        size_t const num_links = tip_node->links().size();
        TRACE_NOMSG(num_links > 1);

        auto path_gen = tip_node->gen_path();

        V3fList points;
        while (not path_gen.is_done()) {
            points.push_back(path_gen.next()->tx_.collapsed());
        }

        DEBUG(points.size() != _.size(),
              "points.size()=%zu, size()=%zu\n",
              points.size(),
              _.size());

        return points;
    }

    /*modifiers */
    void fix_limb_transforms(Link const& arrow) {
        auto limb_gen = arrow.gen_path();
        while (not limb_gen.is_done()) {
            Link const* curr_link = limb_gen.curr_link();
            auto curr_node = limb_gen.curr_node();
            auto next_node = limb_gen.next();

            get_node(next_node)->tx_ = curr_node->tx_ * curr_link->prototype()->tx_;
        }
    }

    NodeKey grow_tip(
        FreeChain const& free_chain_a,
        ProtoLink const* pt_link = nullptr)
    {
        if (not pt_link) {
            pt_link = &free_chain_a.random_proto_link();
        }

        auto node_a = free_chain_a.node;

        // Check that the provided free chain is attached to a node that is a
        // tip node.
        DEBUG_NOMSG(node_a->links().size() > 1);

        auto node_b = _.add_member(
                          pt_link->module_,
                          node_a->tx_ * pt_link->tx_);

        TerminusType const term_a = free_chain_a.term;
        TerminusType const term_b = opposite_term(term_a);

        FreeChain const free_chain_b =
            FreeChain(node_b, term_b, pt_link->chain_id_);

        get_node(node_a)->add_link(free_chain_a, pt_link, free_chain_b);
        get_node(node_b)->add_link(free_chain_b, pt_link->reverse(), free_chain_a);

        _.free_chains_.remove(free_chain_a);
        _.free_chains_.remove(free_chain_b);

        return node_b;
    }

    void nip_tip(
        NodeKey tip_node) {
        // Check node is a tip node.
        size_t const num_links = tip_node->links().size();
        TRACE_NOMSG(num_links != 1);

        NodeKey new_tip = nullptr;

        FreeChain const& new_free_chain =
            begin(tip_node->links())->dst();
        new_tip = new_free_chain.node;

        // Unlink chain.
        get_node(new_tip)->remove_link(new_free_chain);

        // Restore FreeChain.
        _.free_chains_.push_back(new_free_chain);

        _.remove_free_chains(tip_node);
        _.nodes_.erase(tip_node);
    }

    void build_bridge(
        mutation::InsertPoint const& insert_point,
        FreeChain::Bridge const* bridge = nullptr) {
        if (not bridge) {
            bridge = &random::pick(insert_point.bridges);
        }

        FreeChain const& port1 = insert_point.src;
        FreeChain const& port2 = insert_point.dst;
        auto node1 = get_node(port1.node);
        auto node2 = get_node(port2.node);

        // Break link.
        node1->remove_link(port1);
        node2->remove_link(port2);

        // Create a new node in the middle.
        auto new_node = std::make_unique<Node>(
                            bridge->pt_link1->module_,
                            node1->tx_ * bridge->pt_link1->tx_);
        auto new_node_key = new_node.get();
        _.nodes_.emplace(new_node_key, std::move(new_node));

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
            new_node_key, port2.term, bridge->pt_link1->chain_id_);
        Link const new_link1_rev(port1, bridge->pt_link1, nn_src1);

        node1->add_link(new_link1_rev);
        new_node_key->add_link(new_link1_rev.reversed());

        FreeChain nn_src2(
            new_node_key, port1.term, bridge->pt_link2->reverse()->chain_id_);
        Link const new_link2(nn_src2, bridge->pt_link2, port2);

        new_node_key->add_link(new_link2);
        node2->add_link(new_link2.reversed());

        fix_limb_transforms(new_link2);
    }

    void sever_limb(Link const& arrow) {
        // Delete the dst side of arrow.

        Link curr_arrow = arrow;
        size_t num_links = 0;
        do {
            // Unlink nodes.
            NodeKey next_node = curr_arrow.dst().node;
            get_node(next_node)->remove_link(curr_arrow.dst());

            // Check temporary tip node.
            num_links = next_node->links().size();
            DEBUG_NOMSG(num_links > 1); // Must be 0 or 1.

            if (num_links == 1) {
                curr_arrow = *begin(next_node->links());
            }
            else {
                _.remove_free_chains(next_node);
            }

            // Safe to delete node.
            _.nodes_.erase(next_node);
        } while (num_links > 0);

        get_node(arrow.src().node)->remove_link(arrow.src());
        _.free_chains_.push_back(arrow.src());
    }

    // Copy nodes starting from f_arrow.dst to m_arrow.src().
    void copy_limb(
        Link const& m_arrow,
        Link const& f_arrow) {
        DEBUG_NOMSG(_.size() == 0);

        NodeKey tip_node = m_arrow.src().node;
        size_t num_links = tip_node->links().size();
        DEBUG_NOMSG(_.size() > 1 and num_links != 1);

        // Occupy src chain.
        _.free_chains_.remove(m_arrow.src());

        // Form first link.
        ProtoLink const* pt_link =
            tip_node->prototype_->find_link_to(
                m_arrow.src().chain_id,
                m_arrow.src().term,
                f_arrow.dst().node->prototype_,
                f_arrow.dst().chain_id);
        DEBUG_NOMSG(not pt_link);

        tip_node = grow_tip(m_arrow.src(), pt_link);

        num_links = tip_node->links().size();
        DEBUG_NOMSG(num_links != 1);

        auto path_gen = f_arrow.gen_path();
        path_gen.next(); // Same as f_arrow.dst().node.
        while (not path_gen.is_done()) {
            auto curr_link = path_gen.curr_link();
            DEBUG_NOMSG(curr_link->dst().node->prototype_ !=
                        curr_link->prototype()->module_);

            // Modify copy of curr_link->src().
            FreeChain src = curr_link->src();

            DEBUG(src.node->prototype_ != tip_node->prototype_,
                  "%s vs %s\n",
                  src.node->prototype_->name.c_str(),
                  tip_node->prototype_->name.c_str());
            src.node = tip_node;

            // Occupy src chain.
            _.free_chains_.remove(src);
            DEBUG(_.free_chains_.size() != 1,
                  "%zu free chains\n",
                  _.free_chains_.size());

            tip_node = grow_tip(src, curr_link->prototype());
            DEBUG(curr_link->dst().node->prototype_ != tip_node->prototype_,
                  "%s vs %s\n",
                  curr_link->dst().node->prototype_->name.c_str(),
                  tip_node->prototype_->name.c_str());

            num_links = tip_node->links().size();
            DEBUG(num_links != 1,
                  "%zu links\n", num_links);

            path_gen.next();
        }
    }

    /* mutation methods */
    bool erode_mutate() {

        bool mutate_success = false;
#ifndef NO_ERODE
        if (not _.free_chains_.empty()) {
            // Pick random tip node if not specified.
            auto fc_itr = begin(_.free_chains_);
            advance(fc_itr, random::get_dice(_.free_chains_.size()));
            NodeKey tip_node = fc_itr->node;
            _.remove_free_chains(tip_node);

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
                p = p * (_.size() - 2) / (_.size() - 1); // size() is at least 2
                next_loop = random::get_dice_0to1() <= p;

                // Check node is tip node.
                size_t const num_links = tip_node->links().size();
                TRACE_NOMSG(num_links != 1);

                FreeChain const& new_free_chain =
                    begin(tip_node->links())->dst();
                NodeKey new_tip = new_free_chain.node;

                // Unlink
                get_node(new_tip)->remove_link(new_free_chain);

                if (not next_loop) {
                    last_free_chain = new_free_chain;
                }

                _.nodes_.erase(tip_node);

                tip_node = new_tip;
            } while (next_loop);

            // Restore chain.
            _.free_chains_.push_back(last_free_chain);

            regenerate();

            mutate_success = true;
        }
#endif // NO_ERODE

        return mutate_success;
    }

    bool delete_mutate() {

        bool mutate_success = false;
#ifndef NO_DELETE
        if (not _.free_chains_.empty()) {
            // Walk through all nodes to collect delete points.
            std::vector<mutation::DeletePoint> delete_points;

            // Starting at either end is fine.
            auto start_node = begin(_.free_chains_)->node;
            auto path_gen = start_node->gen_path();

            NodeKey curr_node = nullptr;
            auto next_node = path_gen.next(); // Starts with start_node.
            do {
                curr_node = next_node;
                next_node = path_gen.next(); // Can be nullptr.
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

                    auto itr = begin(curr_node->links());
                    Link const& link1 = *itr;
                    advance(itr, 1);
                    Link const& link2 = *itr;
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
                    TRACE("Unexpected num_links",
                          "num_links=%zu\n",
                          num_links);
                }
            } while (not path_gen.is_done());

            // delete_points will at least contain the tip nodes.
            DEBUG_NOMSG(delete_points.empty());

            // Delete a node using a random deletable point.
            auto const& delete_point = random::pick(delete_points);
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
                auto neighbor1 = get_node(delete_point.src.node);
                auto neighbor2 = get_node(delete_point.dst.node);

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
                //         link1->dst     link2->dst
                //                  vvv     vvv
                //  X--[neighbor1]--src->-<-dst
                //                  dst->-<-src--[neighbor2]--...
                //                  ^^^     ^^^
                //         link1->dst     link2->dst
                //

                _.nodes_.erase(delete_point.delete_node);

                // delete_node is guranteed to not be a tip so no need to clean up
                // _.free_chains_.
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
        if (not _.free_chains_.empty()) {
            // Walk through all links to collect insert points.
            std::vector<mutation::InsertPoint> insert_points;

            // Starting at either end is fine.
            auto start_node = begin(_.free_chains_)->node;
            auto path_gen = start_node->gen_path();

            NodeKey curr_node = nullptr;
            auto next_node = path_gen.next(); // Starts with start_node.
            do {
                curr_node = next_node;
                next_node = path_gen.next(); // Can be nullptr.
                size_t const num_links = curr_node->links().size();

                if (num_links == 1) {
                    //
                    // curr_node is a tip node. A new node can be inserted on the
                    // unconnected terminus of a tip node trivially. Use nullptr
                    // in dst().node to flag that this is tip node.
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
                    auto const& ip = insert_points.back();
                    if (ip.bridges.size() == 0) {
                        insert_points.pop_back();
                    }
                }
            } while (not path_gen.is_done());

            // insert_points will at least contain the tip nodes.
            DEBUG_NOMSG(insert_points.empty());

            // Insert a node using a random insert point/
            auto const& insert_point = random::pick(insert_points);
            if (insert_point.dst.node) {
                // This is a non-tip node.
                build_bridge(insert_point);
            }
            else {
                // This is a tip node. Inserting is the same as grow_tip().
                auto fc_itr = find_if(begin(_.free_chains_),
                                      end(_.free_chains_),
                [&](auto const & fc) {
                    return fc.node == insert_point.src.node;
                });

                if (fc_itr == end(_.free_chains_)) {
                    JUtil.error("FreeChain not found for %s\n",
                                insert_point.src.node->to_string().c_str());
                    JUtil.error("Available FreeChain(s):\n");
                    for (FreeChain const& fc : _.free_chains_) {
                        JUtil.error("%s\n", fc.to_string().c_str());
                    }
                    TRACE_NOMSG("FreeChain not found");
                }
                else {
                    grow_tip(*fc_itr);
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
        if (not _.free_chains_.empty()) {
            // Walk through all links to collect swap points.
            std::vector<mutation::SwapPoint> swap_points;

            // Starting at either end is fine.
            auto start_node = begin(_.free_chains_)->node;
            auto path_gen = start_node->gen_path();

            NodeKey prev_node = nullptr;
            NodeKey curr_node = nullptr;
            auto next_node = path_gen.next(); // Starts with start_node/
            do {
                prev_node = curr_node;
                curr_node = next_node;
                next_node = path_gen.next(); // Can be nullptr.
                size_t const num_links = curr_node->links().size();

                if (num_links == 1) {
                    //
                    // curr_node is a tip node. A tip node can be swapped by
                    // deleting it, then randomly growing the tip into a
                    // different ProtoModule. The only time this is not possible
                    // is when the neighbor ProtoModule has no other ProtoLinks
                    // on the terminus in question.
                    //
                    //             X---- [curr_node] -src->-<-dst- [neighbor]
                    //                                         /
                    //                [other choices?] <-------
                    //

                    // Check that neighbor can indead grow into a different
                    // ProtoModule.
                    FreeChain const& tip_fc = begin(curr_node->links())->dst();
                    ProtoModule const* neighbor = tip_fc.node->prototype_;
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
                    auto const& sp = swap_points.back();
                    if (sp.bridges.size() == 0) {
                        swap_points.pop_back();
                    }
                }
            } while (not path_gen.is_done());

            // swap_points may not even contain tip nodes if they can't possibly
            // be swapped.
            if (not swap_points.empty()) {
                // Insert a node using a random insert point.
                auto const& swap_point = random::pick(swap_points);
                if (swap_point.dst.node) {
                    // This is a non-tip node.
                    _.nodes_.erase(swap_point.del_node);
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
        if (not _.free_chains_.empty()) {
            try { // Catch bad cast
                auto& bnt_father = static_cast<PathTeam const&>(father);
                if (not bnt_father.free_chains_.empty()) {
                    // First, collect arrows from both parents.

                    // Starting at either end is fine.
                    auto m_arrows = PathGenerator::collect_arrows(
                                        begin(_.free_chains_)->node);
                    DEBUG_NOMSG(bnt_father.free_chains_.size() != 2);
                    auto f_arrows = PathGenerator::collect_arrows(
                                        begin(bnt_father.free_chains_)->node);

                    // Walk through all link pairs to collect cross points.
                    std::vector<mutation::CrossPoint> cross_points;
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
                        auto const& cp = random::pick(cross_points);

                        Link m_arrow = cp.m_rev ?
                                       cp.m_arrow->reversed() :
                                       *cp.m_arrow; // Make a copy.
                        Link f_arrow = cp.f_rev ?
                                       cp.f_arrow->reversed() :
                                       *cp.f_arrow; // Make a copy.

                        // Always keep m_arrow.src, del m_arrow.dst, and copy from
                        // f_arrow.dst().
                        sever_limb(m_arrow);
                        copy_limb(m_arrow, f_arrow);

                        mutate_success = true;
                    }
                }
            }
            catch (std::bad_cast const& e) {
                PANIC("Bad cast in %s\n", __PRETTY_FUNCTION__);
            }
        }
#endif
        return mutate_success;
    }

    bool regenerate() {
        if (_.nodes_.empty()) {
            // Pick random initial member.
            _.add_member(XDB.basic_mods().draw());
        }

        while (_.size() < _.work_area_->target_size) {
            // Pick next tip chain.
            auto fc_itr = begin(_.free_chains_);
            advance(fc_itr, random::get_dice(_.free_chains_.size()));

            grow_tip(*fc_itr);
        }

        return true;
    }

    bool randomize_mutate() {
        _.nodes_.clear();
        _.free_chains_.clear();
        _.checksum_ = 0x0000;
        _.score_ = INFINITY;
        bool const mutate_success = regenerate();

        TRACE_NOMSG(_.free_chains_.size() != 2); // Replace with mutation_exit_check()

        return mutate_success;
    }

    Node* get_node(NodeKey nk) {
        DEBUG_NOMSG(_.nodes_.find(nk) == end(_.nodes_));
        return _.nodes_.at(nk).get();
    }
};

/* modifiers */
std::unique_ptr<PathTeam::PImpl> PathTeam::make_pimpl() {
    return std::make_unique<PImpl>(*this);
}

/* protected */
/* accessors */
PathTeam* PathTeam::virtual_clone() const {
    return new PathTeam(*this);
}

/* modifiers */
void PathTeam::virtual_copy(NodeTeam const& other) {
    try { // Catch bad cast
        PathTeam::operator=(static_cast<PathTeam const&>(other));
    }
    catch (std::bad_cast const& e) {
        TRACE_NOMSG("Bad cast\n");
    }
}

NodeKey PathTeam::add_member(
    ProtoModule const* const prot,
    Transform const& tx) {
    // We only allow basic modules, which at max can have 2 chains (a 2-termini hub).
    DEBUG_NOMSG(prot->chains().size() == 0 or prot->chains().size() > 2);

    auto new_node = std::make_unique<Node>(prot, tx);
    auto new_node_key = new_node.get();

    for (auto& proto_chain : prot->chains()) {
        if (not proto_chain.n_term().links().empty()) {
            free_chains_.emplace_back(
                new_node_key,
                TerminusType::N,
                proto_chain.id);
        }

        if (not proto_chain.c_term().links().empty()) {
            free_chains_.emplace_back(
                new_node_key,
                TerminusType::C,
                proto_chain.id);
        }
    }

    nodes_.emplace(new_node_key, std::move(new_node));
    return new_node_key;
}

void PathTeam::remove_free_chains(NodeKey const node) {
    // Remove any FreeChain originating from node
    free_chains_.remove_if([&](auto const & fc) {
        return fc.node == node;
    });
}

void PathTeam::calc_checksum() {
    //
    // We want the same checksum for two node teams that consist of the same
    // sequence of nodes even if they are in reverse order. This can be
    // achieved by XOR'ing the forward and backward checksums.
    //
    DEBUG(free_chains_.size() != 2,
          "There are %zu free chains!",
          free_chains_.size());
    DEBUG_NOMSG(size() == 0);

    checksum_ = 0x0000;
    for (auto& free_chain : free_chains_) {
        Crc32 const crc_half = PathGenerator::path_checksum(free_chain.node);

        Crc32 tmp = checksum_ ^ crc_half;
        if (not tmp) {
            // If result is 0x0000, it's due to the node sequence being
            // symmetrical. No need to compute the other direction if it's
            // symmetrical.
            break;
        }

        checksum_ = tmp;
    }
}

void PathTeam::calc_score() {
    //
    // We score the path forward and backward, because the Kabsch
    // algorithm relies on point-wise correspondance. Different ordering
    // can yield different RMSD scores.
    //
    DEBUG(free_chains_.size() != 2,
          "There are %zu free chains!\n",
          free_chains_.size());
    DEBUG_NOMSG(size() == 0);

    score_ = INFINITY;
    for (auto& free_chain : free_chains_) {
        auto const my_points = pimpl_->collect_points(free_chain.node);
        float const new_score =
            kabsch::score(my_points,
                          work_area_->points);

        if (new_score < score_) {
            score_ = new_score;
            scored_tip_ = free_chain.node;
        }
    }
}

/* public */
/* ctors */
PathTeam::PathTeam(WorkArea const* wa) :
    NodeTeam(wa),
    pimpl_(make_pimpl())
{
    TRACE_NOMSG(not work_area_);
}

PathTeam::PathTeam(WorkArea const* wa, tests::Recipe const& recipe) :
    PathTeam(wa)
{
    if (not recipe.empty()) {
        std::string const& first_mod_name = recipe[0].mod_name;
        auto last_node = add_member(XDB.get_module(first_mod_name));

        for (auto itr = begin(recipe); itr < end(recipe) - 1; ++itr) {
            auto const& step = *itr;

            // Now create next node.
            auto src_mod = XDB.get_module(step.mod_name);
            auto dst_mod = XDB.get_module((itr + 1)->mod_name);
            TRACE_NOMSG(not src_mod);
            TRACE_NOMSG(not dst_mod);

            size_t const src_chain_id = src_mod->find_chain_id(step.src_chain);
            size_t const dst_chain_id = dst_mod->find_chain_id(step.dst_chain);

            // Find ProtoLink
            auto pt_link = src_mod->find_link_to(
                               src_chain_id,
                               step.src_term,
                               dst_mod,
                               dst_chain_id);

            // Find FreeChain
            TRACE_NOMSG(free_chains_.size() != 2);
            FreeChain src_fc(last_node, step.src_term, src_chain_id);

            TRACE_NOMSG(std::find(begin(free_chains_),
                                  end(free_chains_),
                                  src_fc) == end(free_chains_));

            last_node = pimpl_->grow_tip(src_fc, pt_link);
        }

        calc_checksum();
    }
}

PathTeam::PathTeam(PathTeam const& other) :
    PathTeam(other.work_area_)
{
    *this = other; // Calls operator=(T const&)
}

PathTeam::PathTeam(PathTeam&& other) :
    PathTeam(other.work_area_)
{
    *this = std::move(other); // Calls operator=(T&&)
}

/* dtors */
PathTeam::~PathTeam() {}

/* accessors */
PathGenerator PathTeam::gen_path() const {
    DEBUG(not scored_tip_, "calc_score() not called\n");
    return scored_tip_->gen_path();
}

// NodeKey PathTeam::pick_tip_node() const {

// }

// void PathTeam::mutation_invariant_check() const {

// }

/* modifiers */
PathTeam& PathTeam::operator=(PathTeam const& other) {
    if (this != &other) {
        nodes_.clear();
        free_chains_.clear();

        NodeTeam::operator=(other);

        // Clone nodes and create address mapping for remapping pointers.
        {
            NodeKeyMap nk_map; // old addr -> new addr

            for (auto& other_node_itr : other.nodes_) {
                auto& other_node = other_node_itr.second;
                NodeSP node = other_node->clone();
                nk_map[other_node.get()] = node.get();
                nodes_.emplace(node.get(), std::move(node));
            }

            free_chains_ = other.free_chains_;

            // Fix pointer addresses and assign to my own nodes
            for (auto& node_itr : nodes_) {
                auto& node = node_itr.second;
                node->update_link_ptrs(nk_map);
            }

            for (auto& fc : free_chains_) {
                fc.node = nk_map.at(fc.node);
            }

            if (other.scored_tip_) {
                scored_tip_ = nk_map.at(other.scored_tip_);
            }
        }
    }

    return *this;
}

PathTeam& PathTeam::operator=(PathTeam&& other) {
    if (this != &other) {
        nodes_.clear();
        free_chains_.clear();

        NodeTeam::operator=(std::move(other));

        std::swap(nodes_, other.nodes_);
        std::swap(free_chains_, other.free_chains_);
        std::swap(scored_tip_, other.scored_tip_);
    }

    return *this;
}

void PathTeam::randomize() {
    pimpl_->randomize_mutate();
}

mutation::Mode PathTeam::evolve(
    NodeTeam const& mother,
    NodeTeam const& father)
{
    virtual_copy(mother);

    auto modes = mutation::gen_mode_list();

    bool mutate_success = false;
    mutation::Mode mode;

    while (not mutate_success and not modes.empty()) {
        TRACE_NOMSG(size() == 0);
        TRACE_NOMSG(free_chains_.size() != 2); // Replace with mutation_entry_check()

        mode = random::pop(modes);
        switch (mode) {
        case mutation::Mode::ERODE:
            mutate_success = pimpl_->erode_mutate();
            break;
        case mutation::Mode::DELETE:
            mutate_success = pimpl_->delete_mutate();
            break;
        case mutation::Mode::INSERT:
            mutate_success = pimpl_->insert_mutate();
            break;
        case mutation::Mode::SWAP:
            mutate_success = pimpl_->swap_mutate();
            break;
        case mutation::Mode::CROSS:
            mutate_success = pimpl_->cross_mutate(father);
            break;
        case mutation::Mode::REGENERATE:
            mutate_success = pimpl_->regenerate();
            break;
        default:
            mutation::bad_mode(mode);
        }

        TRACE_NOMSG(free_chains_.size() != 2); // Replace with mutation_exit_check()
    }

    if (not mutate_success) {
        mutate_success = pimpl_->randomize_mutate();
        TRACE_NOMSG(not mutate_success);
    }

    calc_checksum();
    calc_score();

    return mode;
}

/* printers */
void PathTeam::print_to(std::ostream& os) const {
    TRACE_NOMSG(free_chains_.empty());
    auto start_node = begin(free_chains_)->node;
    auto path_gen = start_node->gen_path();

    while (not path_gen.is_done()) {
        os << path_gen.next()->to_string();
        Link const* link_ptr = path_gen.curr_link();
        if (link_ptr) {
            TerminusType const src_term = link_ptr->src().term;
            TerminusType const dst_term = link_ptr->dst().term;
            os << "\n(" << TerminusTypeToCStr(src_term) << ", ";
            os << TerminusTypeToCStr(dst_term) << ")\n";
        }
    }
}

JSON PathTeam::to_json() const {
    JSON output;

    if (not free_chains_.empty()) {
        DEBUG_NOMSG(free_chains_.size() != 2);

        // Kabsch outputs for forward and backward paths.
        elfin::Mat3f rot[2];
        Vector3f tran[2];
        float rms[2] = { 0 };

        auto fc_itr = begin(free_chains_);
        for (size_t i = 0; i < 2; ++i) {
            advance(fc_itr, i);
            V3fList const& points = pimpl_->collect_points(fc_itr->node);

            kabsch::calc_alignment(
                /*mobile=*/ points,
                /*ref=*/ work_area_->points,
                rot[i],
                tran[i],
                rms[i]);
        }

        // Start at tip that yields lower score.
        size_t const better_tip_id = rms[0] < rms[1] ? 0 : 1;

        fc_itr = begin(free_chains_);
        advance(fc_itr, better_tip_id);
        NodeKey tip_node = fc_itr->node;
        Transform kabsch_alignment(rot[better_tip_id], tran[better_tip_id]);

        size_t member_id = 0;  // UID for node in team.
        auto path_gen = tip_node->gen_path();
        while (not path_gen.is_done()) {
            auto curr_node = path_gen.next();

            JSON node_output;
            node_output["name"] = curr_node->prototype_->name;
            node_output["member_id"] = member_id;

            auto link = path_gen.curr_link();
            if (link) {  //  Not reached end of nodes yet.
                node_output["src_term"] =
                    TerminusTypeToCStr(link->src().term);

                std::string const& src_chain_name =
                    curr_node->prototype_->chains().at(
                        link->src().chain_id).name;
                std::string const& dst_chain_name =
                    path_gen.peek()->prototype_->chains().at(
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
              "output.size()=%zu, size()=%zu\n",
              output.size(),
              this->size());
    }

    return output;
}

}  /* elfin */