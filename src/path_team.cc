#include "path_team.h"

#include "scoring.h"
#include "path_generator.h"
#include "input_manager.h"
#include "id_types.h"
#include "mutation.h"

namespace elfin {

/* private */
struct PathTeam::PImpl {
    /* data */
    PathTeam& _;

    /* ctors */
    PImpl(PathTeam& interface) : _(interface) {}

    /*modifiers */
    void add_free_terms(NodeKey const node_key,
                        size_t const n_ft_to_add,
                        FreeTerm const* const exclude_ft) {
        auto const prot = node_key->prototype_;
        auto prot_free_terms = prot->free_terms();

        {
            size_t const n_prot_free_terms = prot_free_terms.size();
            DEBUG(n_prot_free_terms < 2,
                  "Dead end (%zu free terms) ProtoModule detected: %s\n",
                  n_prot_free_terms, prot->name.c_str());
        }

        size_t count_down = n_ft_to_add;
        while (count_down) {
            DEBUG(prot_free_terms.empty(),
                  "%s has insufficient active ProtoTerms! "
                  "Tried to add %zu, prot had %zu free terms. exclude_ft: %p\n",
                  prot->name.c_str(),
                  n_ft_to_add,
                  prot->free_terms().size(),
                  exclude_ft);

            auto ft = random::pop(prot_free_terms);

            if (not (exclude_ft and
                     exclude_ft->chain_id == ft.chain_id and
                     exclude_ft->term == ft.term))
            {
                _.free_terms_.emplace_back(node_key,
                                           ft.chain_id,
                                           ft.term);
                count_down--;
            }
        }
    }

    Node* get_node(NodeKey const nk) {
        DEBUG_NOMSG(_.nodes_.find(nk) == end(_.nodes_));
        return _.nodes_.at(nk).get();
    }

    void nip_tip(NodeKey tip_node)
    {
        // Check node is a tip node.
        size_t const num_links = tip_node->links().size();
        TRACE_NOMSG(num_links != 1);

        FreeTerm const& new_free_term =
            begin(tip_node->links())->dst();
        NodeKey new_tip = new_free_term.node;

        // Unlink chain.
        get_node(new_tip)->remove_link(new_free_term);

        // Restore FreeTerm.
        if (new_free_term.should_restore) {
            _.free_terms_.push_back(new_free_term);
        }

        // Remove tip node. Don't do this before restoring the FreeTerm
        // unless new_free_term is a copy rather than a reference.
        _.remove_free_terms(tip_node);
        _.nodes_.erase(tip_node);
    }

    void fix_limb_transforms(Link const& arrow)
    {
        PathGenerator limb_gen(&arrow);
        while (not limb_gen.is_done()) {
            Link const* curr_link = limb_gen.curr_link();
            auto curr_node = limb_gen.curr_node();
            auto next_node = limb_gen.next();

            get_node(next_node)->tx_ = curr_node->tx_ * curr_link->prototype()->tx;
        }
    }

    void build_bridge(mutation::InsertPoint const& insert_point,
                      FreeTerm::Bridge const* bridge = nullptr)
    {
        if (not bridge) {
            bridge = &random::pick(insert_point.bridges);
        }

        FreeTerm const& port1 = insert_point.src;
        FreeTerm const& port2 = insert_point.dst;
        auto node1 = get_node(port1.node);
        auto node2 = get_node(port2.node);

        // Break link.
        node1->remove_link(port1);
        node2->remove_link(port2);

        // Create a new node in the middle.
        auto new_node_key = _.add_node(bridge->pt_link1->module,
                                       node1->tx_ * bridge->pt_link1->tx,
                                       /*innert=*/true);
        auto new_node = get_node(new_node_key);

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
        FreeTerm nn_src1(
            new_node_key, bridge->pt_link1->chain_id, port2.term);
        Link const new_link1_rev(port1, bridge->pt_link1, nn_src1);

        node1->add_link(new_link1_rev);
        new_node->add_link(new_link1_rev.reversed());

        FreeTerm nn_src2(
            new_node_key, bridge->pt_link2->reverse->chain_id, port1.term);
        Link const new_link2(nn_src2, bridge->pt_link2, port2);

        new_node->add_link(new_link2);
        node2->add_link(new_link2.reversed());

        fix_limb_transforms(new_link2);
    }

    void sever_limb(Link const& arrow) {
        // Delete the dst side of arrow.

        auto start_node_key = arrow.dst().node;
        get_node(start_node_key)->remove_link(arrow.dst());

        PathGenerator pg(start_node_key);
        while (not pg.is_done()) {
            auto next_key = pg.next();
            _.nodes_.erase(next_key);
        }

        _.remove_free_terms(pg.curr_node());

        get_node(arrow.src().node)->remove_link(arrow.src());

        auto const& would_be_free = arrow.src();
        if (would_be_free.should_restore) {
            _.free_terms_.push_back(would_be_free);
        }
    }

    // Copy nodes starting from f_arrow.dst to m_free_term.
    void copy_limb(FreeTerm const& m_free_term,
                   ProtoLink const* cross_link,
                   Link const& f_arrow)
    {
        PathGenerator path_gen(&f_arrow);

        // Form first link. It's special because m_free_term may not equal
        // f_arrow.src(). It just happens that m_free_term's ProtoModule has a
        // ProtoLink to f_arrow.src()'s.
        path_gen.next();  // Advance past the non identical arrow.

        auto tip_node = _.grow_tip(m_free_term,
                                   cross_link,
                                   /*innert=*/path_gen.peek());

        while (not path_gen.is_done()) {
            auto curr_link = path_gen.curr_link();

            // Modify copy of curr_link->src().
            FreeTerm src = curr_link->src();

            DEBUG(src.node->prototype_ != tip_node->prototype_,
                  "%s vs %s\n",
                  src.node->prototype_->name.c_str(),
                  tip_node->prototype_->name.c_str());
            src.node = tip_node;

            path_gen.next();

            tip_node = _.grow_tip(src,
                                  curr_link->prototype(),
                                  /*innert=*/path_gen.peek());

            DEBUG(curr_link->dst().node->prototype_ != tip_node->prototype_,
                  "%s vs %s\n",
                  curr_link->dst().node->prototype_->name.c_str(),
                  tip_node->prototype_->name.c_str());
        }
    }

    /* mutation methods */
    bool erode_mutate()
    {
        bool mutate_success = false;

        if (_.size() > 1) {
            // Pick random tip node if not specified.
            NodeKey tip_node = _.get_tip(/*mutable_hint=*/true);
            _.remove_free_terms(tip_node);

            FreeTerm last_free_term;
            float p = 1.0f;  // p for Probability.

            // Loop condition is always true on first entrance, hence do-while.
            bool next_loop = false;
            size_t const original_size = _.size();
            do {
                // Calculate next iteration condition - linearly falling
                // probability.
                p = (_.size() - 1) / original_size;
                next_loop = random::get_dice_0to1() <= p;

                // Check node is tip node.
                size_t const num_links = tip_node->links().size();
                TRACE(num_links != 1, "num_links=%zu", num_links);

                FreeTerm const& new_free_term =
                    begin(tip_node->links())->dst();
                NodeKey new_tip = new_free_term.node;

                // Unlink
                get_node(new_tip)->remove_link(new_free_term);

                if (not next_loop) {
                    last_free_term = new_free_term;
                }

                _.nodes_.erase(tip_node);

                tip_node = new_tip;
            } while (next_loop);

            // Restore chain.
            if (last_free_term.should_restore) {
                _.free_terms_.push_back(last_free_term);
            }

            regenerate();

            mutate_success = true;
        }

        return mutate_success;
    }

    bool delete_mutate()
    {
        bool mutate_success = false;

        if (_.size() > 1) {
            // Walk through all nodes to collect delete points.
            std::vector<mutation::DeletePoint> delete_points;

            // Starting at either end is fine.
            auto start_node = begin(_.free_terms_)->node;
            PathGenerator path_gen(start_node);

            NodeKey curr_node = nullptr;
            auto next_node = path_gen.next();  // Starts with start_node.
            do {
                curr_node = next_node;
                next_node = path_gen.next();  // Can be nullptr.
                size_t const num_links = curr_node->links().size();

                if (num_links == 1) {
                    if ( _.is_mutable(curr_node)) {
                        //
                        // curr_node is a tip node, which can always be deleted trivially.
                        // Use ProtoLink* = nullptr to mark a tip node. Pointers
                        // src and dst are not used.
                        //
                        delete_points.emplace_back(
                            /*delete_node=*/ curr_node,
                            /*src=*/ FreeTerm(),
                            /*dst=*/ FreeTerm(),
                            /*skipper=*/ nullptr);
                    }
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
                    FreeTerm const& src = link1.dst();
                    FreeTerm const& dst = link2.dst();

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

            // delete_points might be empty (if HingeTeam has no other nodes
            // than hinge_).
            if (not delete_points.empty()) {
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
                    // _.free_terms_.
                    fix_limb_transforms(arrow1);
                }
                else {
                    // This is a tip node.
                    nip_tip(delete_point.delete_node);
                }

                mutate_success = true;
            }
        }

        return mutate_success;
    }

    bool insert_mutate()
    {
        bool mutate_success = false;

        // Walk through all links to collect insert points.
        std::vector<mutation::InsertPoint> insert_points;

        auto start_node = _.get_tip(/*mutable_hint=*/false);
        PathGenerator path_gen(start_node);

        NodeKey curr_node = nullptr;
        auto next_node = path_gen.next();
        do {
            curr_node = next_node;
            next_node = path_gen.next();  // Can be nullptr.
            size_t const num_links = curr_node->links().size();

            if (num_links == 1 and _.is_mutable(curr_node)) {
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

                insert_points.emplace_back(
                    FreeTerm(curr_node, 0, TermType::NONE),
                    FreeTerm());
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
                if (ip.bridges.empty()) {
                    insert_points.pop_back();
                }
            }
        } while (not path_gen.is_done());

        // insert_points might be empty (if HingeTeam has no other nodes
        // than hinge_).
        if (not insert_points.empty()) {
            // Insert a node using a random insert point
            auto const& insert_point = random::pick(insert_points);
            if (insert_point.dst.node) {
                // This is a non-tip node.
                build_bridge(insert_point);
            }
            else {
                // This is a tip node. Inserting is the same as grow_tip().
                auto ft_itr = find_if(begin(_.free_terms_),
                                      end(_.free_terms_),
                [&](auto const & ft) {
                    return ft.node == insert_point.src.node;
                });

                if (ft_itr == end(_.free_terms_)) {
                    std::ostringstream oss;
                    oss << "FreeTerm not found for ";
                    oss << *insert_point.src.node << "\n";

                    oss << "Available FreeTerm(s):\n";
                    for (auto const& ft : _.free_terms_) {
                        oss << ft << "\n";
                    }
                    throw ValueNotFound(oss.str());
                }
                else {
                    _.grow_tip(*ft_itr);
                }
            }

            mutate_success = true;
        }

        return mutate_success;
    }

    bool swap_mutate()
    {
        bool mutate_success = false;

        // Walk through all links to collect swap points.
        std::vector<mutation::SwapPoint> swap_points;

        auto start_node = _.get_tip(/*mutable_hint=*/false);
        PathGenerator path_gen(start_node);

        NodeKey prev_node = nullptr;
        NodeKey curr_node = nullptr;
        auto next_node = path_gen.next();  // Starts with start_node/
        do {
            prev_node = curr_node;
            curr_node = next_node;
            next_node = path_gen.next();  // Can be nullptr.
            size_t const num_links = curr_node->links().size();

            if (num_links == 1 and _.is_mutable(curr_node)) {
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
                FreeTerm const& tip_ft = begin(curr_node->links())->dst();
                ProtoModule const* neighbor = tip_ft.node->prototype_;
                ProtoChain const& chain = neighbor->chains().at(tip_ft.chain_id);

                if (chain.get_term(tip_ft.term).links().size() > 1) {
                    swap_points.emplace_back(tip_ft, curr_node, FreeTerm());
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
                if (sp.bridges.empty()) {
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
                _.grow_tip(swap_point.src);
            }

            mutate_success = true;
        }

        return mutate_success;
    }

    bool cross_mutate(NodeTeam const& father)
    {
        bool mutate_success = false;

        try { // Catch bad cast
            auto& pt_father = static_cast<PathTeam const&>(father);

            // First, collect arrows from both parents.
            auto mother_tip = _.get_tip(/*mutable_hint=*/false);
            auto m_arrows = PathGenerator(mother_tip).collect_arrows();

            auto father_tip = pt_father.get_tip(/*mutable_hint=*/false);
            auto f_arrows = PathGenerator(father_tip).collect_arrows();

            // Walk through all link pairs to collect cross points.
            std::vector<mutation::CrossPoint> cross_points;
            for (Link const* m_arrow : m_arrows) {
                for (Link const* f_arrow : f_arrows) {
                    ProtoLink const* sd =
                        m_arrow->src().find_link_to(f_arrow->dst());
                    if (sd) {
                        cross_points.emplace_back(
                            sd,
                            m_arrow,   // { } --m_arrow--v { del  }
                            f_arrow);  // { } --f_arrow--> { copy }
                    }
                }
            }

            if (not cross_points.empty()) {
                auto const& cp = random::pick(cross_points);

                // Make copies.
                Link m_arrow = *cp.m_arrow;
                Link f_arrow = *cp.f_arrow;

                // Always keep m_arrow.src, del m_arrow.dst, and copy from
                // f_arrow.dst().
                sever_limb(m_arrow);
                copy_limb(m_arrow.src(), cp.pt_link, f_arrow);

                mutate_success = true;
            }
        }
        catch (std::bad_cast const& e) {
            PANIC(e);
        }

        return mutate_success;
    }

    bool regenerate() {
        if (_.nodes_.empty()) {
            // Pick random initial member.
            _.add_node(XDB.basic_mods().draw());
        }

        while (_.size() < _.work_area_->target_size) {
            _.grow_tip(_.get_mutable_term());
        }

        return true;
    }

    void randomize() {
        _.reset();
        regenerate();
        _.mutation_invariance_check();
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

FreeTerm PathTeam::get_mutable_term() const
{
    return random::pick(free_terms_);
}

NodeKey PathTeam::get_tip(bool const mutable_hint) const {
    return get_mutable_term().node;
}

void PathTeam::mutation_invariance_check() const {
    if (free_terms_.size() != 2) {
        for (auto& ft : free_terms_) {
            JUtil.warn("ft: %s\n", ft.to_string().c_str());
        }
    }
    DEBUG(free_terms_.size() != 2,
          "free_terms_.size()=%zu\n", free_terms_.size());
    DEBUG_NOMSG(size() == 0);
}

bool PathTeam::is_mutable(NodeKey const tip) const {
    return true;
}

void PathTeam::postprocess_json(JSON& output) const {}

/* modifiers */
void PathTeam::reset() {
    NodeTeam::reset();
    nodes_.clear();
    free_terms_.clear();
    scored_path_ = nullptr;
    nk_map_.clear();
}

void PathTeam::virtual_copy(NodeTeam const& other) {
    try { // Catch bad cast
        PathTeam::operator=(static_cast<PathTeam const&>(other));
    }
    catch (std::bad_cast const& e) {
        TRACE_NOMSG("Bad cast\n");
    }
}

NodeKey PathTeam::add_node(ProtoModule const* const prot,
                           Transform const& tx,
                           bool const innert,
                           size_t const n_ft_to_add,
                           FreeTerm const* const exclude_ft)
{
    auto new_node = std::make_unique<Node>(prot, tx);
    auto new_node_key = new_node.get();

    nodes_.emplace(new_node_key, std::move(new_node));

    if (not innert) {
        pimpl_->add_free_terms(new_node_key, n_ft_to_add, exclude_ft);
    }

    return new_node_key;
}

NodeKey PathTeam::grow_tip(FreeTerm const& free_term_a,
                           ProtoLink const* pt_link,
                           bool const innert)
{
    if (not pt_link) {
        pt_link = &free_term_a.random_proto_link();
    }

    auto node_a = free_term_a.node;

    // Check that the provided free term is attached to a node that is a
    // tip node.
    {
        size_t const n_links = node_a->links().size();
        DEBUG(n_links > 1, "%zu\n", n_links);
    }

    TermType const term_a = free_term_a.term;
    TermType const term_b = opposite_term(term_a);

    FreeTerm free_term_b = FreeTerm(nullptr, pt_link->chain_id, term_b);

    auto node_b = add_node(pt_link->module,
                           node_a->tx_ * pt_link->tx,
                           innert,
                           /*n_ft_to_add=*/ 1,
                           /*exclude_ft=*/ &free_term_b);

    free_term_b.node = node_b;

    pimpl_->get_node(node_a)->add_link(free_term_a, pt_link, free_term_b);
    pimpl_->get_node(node_b)->add_link(free_term_b, pt_link->reverse, free_term_a);

    free_terms_.remove(free_term_a);

    // Check that newly grown node is a tip node.
    {
        size_t const n_links = node_b->links().size();
        DEBUG(n_links > 1, "%zu\n", n_links);
    }

    return node_b;
}

void PathTeam::remove_free_terms(NodeKey const node) {
    // Remove any FreeTerm originating from node
    free_terms_.remove_if([&](auto const & ft) {
        return ft.node == node;
    });
}

void PathTeam::evavluate() {
    calc_checksum();
    calc_score();
}

void PathTeam::calc_checksum() {
    // We want the same checksum for two node teams that consist of the same
    // sequence of nodes even if they are in reverse order. This can be
    // achieved by XOR'ing the forward and backward checksums.
    checksum_ = 0x0000;
    for (auto& free_term : free_terms_) {
        Crc32 const crc_half =
            PathGenerator(free_term.node).checksum();

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
    // Score the path forward and backward, because the Kabsch
    // algorithm relies on point-wise correspondance. Different ordering
    // can yield different RMSD scores.
    score_ = INFINITY;

    auto tip = get_tip(/*mutable_hint=*/false);
    auto const& my_points = PathGenerator(tip).collect_points();

    auto const& [fwd_ui_key, fwd_path] = *begin(work_area_->path_map);
    float const fwd_score = scoring::score_aligned(my_points, fwd_path);

    auto const& [bwd_ui_key, bwd_path] = *(++begin(work_area_->path_map));
    float const bwd_score = scoring::score_aligned(my_points, bwd_path);

    if (fwd_score < bwd_score) {
        score_ = fwd_score;
        scored_path_ = &fwd_path;
    }
    else {
        score_ = bwd_score;
        scored_path_ = &bwd_path;
    }
}

void PathTeam::virtual_implement_recipe(tests::Recipe const& recipe,
                                        FirstLastNodeKeyCallback const& postprocessor,
                                        Transform const& shift_tx)
{

    TRACE_NOMSG(recipe.empty());

    // Let derived classes do their own partial reset.
    PathTeam::reset();

    NodeKey first_node = nullptr;
    if (not recipe.empty()) {
        std::string const& first_mod_name = recipe[0].mod_name;

        // Call grow_tip() with innert=true until last node, because we want
        // to trust the recipe being correct. If hubs are involved, non-innert
        // mode will choose only 2 random FreeTerms to add to free_terms_.
        first_node = add_node(XDB.get_mod(first_mod_name), shift_tx, /*innert=*/ true);

        auto last_node = first_node;

        for (auto itr = begin(recipe); itr < end(recipe) - 1; ++itr) {
            auto const& step = *itr;

            // Create next node.
            auto src_mod = XDB.get_mod(step.mod_name);
            auto dst_mod = XDB.get_mod((itr + 1)->mod_name);
            TRACE_NOMSG(not src_mod);
            TRACE_NOMSG(not dst_mod);

            size_t const src_chain_id = src_mod->get_chain_id(step.src_chain);
            size_t const dst_chain_id = dst_mod->get_chain_id(step.dst_chain);

            // Find ProtoLink.
            auto pt_link = src_mod->find_link_to(
                               src_chain_id,
                               step.src_term,
                               dst_mod,
                               dst_chain_id);

            FreeTerm const src_ft(last_node, src_chain_id, step.src_term);

            last_node = grow_tip(src_ft, pt_link, /*innert=*/ true);
        }

        // Because we called grow_tip() with innert=true, now we need to add back
        // the free terms that would've been added with innert=false.
        auto const compensate_free_terms = [&](NodeKey const tip_node) {
            DEBUG_NOMSG(tip_node->links().size() != 1);
            auto const& link = *begin(tip_node->links());
            pimpl_->add_free_terms(tip_node,
                                   /*n_ft_to_add=*/ 1,
                                   /*exclude_ft=*/ &link.src());
        };
        compensate_free_terms(first_node);
        if (first_node != last_node)
            compensate_free_terms(last_node);

        if (postprocessor) {
            postprocessor(first_node, last_node);
        }
    }
}

/* public */
/* ctors */
PathTeam::PathTeam(WorkArea const* wa) :
    NodeTeam(wa),
    pimpl_(make_pimpl()) {}

PathTeam::PathTeam(PathTeam const& other) :
    PathTeam(other.work_area_)
{ this->operator=(other); }

PathTeam::PathTeam(PathTeam&& other) :
    PathTeam(other.work_area_)
{ this->operator=(std::move(other)); }

/* dtors */
PathTeam::~PathTeam() {}

/* accessors */
PathGenerator PathTeam::gen_path() const {
    return PathGenerator(get_tip(/*mutable_hint=*/false));
}

/* modifiers */
PathTeam& PathTeam::operator=(PathTeam const& other) {
    if (this != &other) {
        NodeTeam::operator=(other);

        // Clone nodes and create address mapping for remapping pointers.
        {
            // Create new node addr mapping: other addr -> my addr
            nk_map_.clear();
            nodes_.clear();

            for (auto& [other_nk, other_node] : other.nodes_) {
                NodeSP my_node = other_node->clone();
                nk_map_[other_nk] = my_node.get();
                nodes_.emplace(my_node.get(), std::move(my_node));
            }

            // Fix pointer addresses and assign to my own nodes.
            for (auto& [nk, node] : nodes_) {
                node->update_link_ptrs(nk_map_);
            }

            // Copy free_terms_.
            free_terms_ = other.free_terms_;
            for (auto& ft : free_terms_) {
                ft.node = nk_map_.at(ft.node);
            }

            scored_path_ = other.scored_path_;
        }
    }

    return *this;
}

PathTeam& PathTeam::operator=(PathTeam&& other) {
    if (this != &other) {
        NodeTeam::operator=(std::move(other));

        std::swap(nodes_, other.nodes_);
        std::swap(free_terms_, other.free_terms_);
        std::swap(scored_path_, other.scored_path_);
        std::swap(nk_map_, other.nk_map_);
    }

    return *this;
}

void PathTeam::randomize() {
    pimpl_->randomize();
    evavluate();
}

mutation::Mode PathTeam::evolve(NodeTeam const& mother,
                                NodeTeam const& father)
{
    virtual_copy(mother);

    auto modes = mutation::gen_mode_list();

    bool mutate_success = false;
    mutation::Mode mode = mutation::Mode::NONE;

    while (not mutate_success and not modes.empty()) {
        mutation_invariance_check();

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

        mutation_invariance_check();
    }

    if (not mutate_success) {
        pimpl_->randomize();
        mutate_success = true;
    }

    evavluate();

    return mode;
}

/* printers */
void PathTeam::print_to(std::ostream& os) const {
    TRACE_NOMSG(free_terms_.empty());

    os << "PathTeam[\n";

    auto start_node = get_tip(/*mutable_hint=*/false);
    PathGenerator path_gen(start_node);
    while (not path_gen.is_done()) {
        os << *path_gen.next();
        Link const* link_ptr = path_gen.curr_link();
        if (link_ptr) {
            os << "  src: " << link_ptr->src() << "\n";
            os << "  dst: " << link_ptr->dst() << "\n";
        }
    }

    os << "]PathTeam\n";
}

JSON PathTeam::to_json() const {
    JSON output;

    if (not free_terms_.empty()) {
        // Compute Kabsch alignment.
        elfin::Mat3f rot;
        Vector3f tran;

        {
            float rms = INFINITY;
            V3fList const& points = gen_path().collect_points();
            scoring::calc_alignment(
                /*mobile=*/ points, /*ref=*/ *scored_path_, rot, tran, rms);
        }

        Transform kabsch_alignment(rot, tran);

        size_t member_id = 0;  // UID for node in team.
        auto pg = gen_path();
        while (not pg.is_done()) {
            auto node_key = pg.next();

            JSON node_output;
            try {
                node_output["name"] = node_key->prototype_->name;
                node_output["member_id"] = member_id;

                auto link = pg.curr_link();
                if (link) {  //  Not reached end of nodes yet, so peek() != nullptr.
                    node_output["src_term"] =
                        TermTypeToCStr(link->src().term);

                    node_output["src_chain_name"] =
                        node_key->prototype_->chains().at(
                            link->src().chain_id).name;

                    node_output["dst_chain_name"] =
                        pg.peek()->prototype_->chains().at(
                            link->dst().chain_id).name;
                }
                else
                {
                    node_output["src_term"] =
                        TermTypeToCStr(TermType::NONE);
                    node_output["src_chain_name"] = "NONE";
                    node_output["dst_chain_name"] = "NONE";
                }

                Transform const tx = kabsch_alignment * node_key->tx_;

                node_output["rot"] = tx.rot_json();
                node_output["tran"] = tx.tran_json();
            } catch (JSON::exception const& je) {
                JUtil.error("Here!\n");
                JSON_LOG_EXIT(je);
            }

            member_id++;
            output.emplace_back(node_output);
        }

        DEBUG(output.size() != size(),
              "output.size()=%zu, size()=%zu\n",
              output.size(),
              this->size());
    }

    postprocess_json(output);

    return output;
}

}  /* elfin */