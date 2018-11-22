#include "node_list.h"

#include "database.h"
#include "id_pair.h"

#define NO_POINT_MUTATE
#define NO_LIMB_MUTATE
#define NO_CROSS_MUTATE

#if defined(NO_POINT_MUTATE) || \
    defined(NO_LIMB_MUTATE) || \
    defined(NO_CROSS_MUTATE)
#warning "Mutation code not being compiled!!"
#endif

namespace elfin {

/* static data members */
size_t NodeList::MAX_LEN_ = 0;
const size_t & NodeList::MAX_LEN = NodeList::MAX_LEN_;

/*
 * Checks whether new_com is too close to any other com.
 */
bool NodeList::collides(
    const Vector3f & new_com,
    const float mod_radius) const {

    for (const auto & node : *this) {
        const float sq_com_dist = node.tx.collapsed().sq_dist_to(new_com);
        const float required_com_dist = mod_radius +
                                        node.prototype->radius_;
        if (sq_com_dist < (required_com_dist * required_com_dist))
            return true;
    }

    return false;
}

/*
 * Finds points at which a severance would allow a different limb to be grown.
 * (Modifies "id_out")
 */
bool NodeList::pick_sever_point(
    size_t & id_out,
    const TerminusType term) {

    bool success = false;
    std::vector<size_t> non_dead_end_ids;

    for (size_t i = 0; i < this->size(); ++i) {
        const Module * proto = this->at(i).prototype;
        if ((term == N and proto->n_link_count() > 1) or
                (term == C and proto->c_link_count() > 1) or
                (term == ANY and proto->all_link_count() > 2)) {

            non_dead_end_ids.push_back(i);

        }
    }

    if (!non_dead_end_ids.empty()) {
        id_out = pick_random(non_dead_end_ids);
        success = true;
    }

    return success;
}

/*
 * Grows a random limb of length "n_nodes" starting from "head" at "term"
 * terminus.
 */
void NodeList::randomize(
    NodeList::const_iterator head,
    const TerminusType term,
    const size_t n_nodes) {

    die("Not rewritten\n");

    // Cut limb

    // if (nodes.empty()) {
    //     // Pick random starting node
    //     nodes.emplace_back(XDB.get_rand_mod(), Transform());
    // }

    // while (nodes.size() <= max_len) {
    //     // Find list of non-colliding neighbour modules
    //     std::vector<const Module::Link *> compat_links;
    //     const Transform & base_tx = nodes.back().tx;
    //     const Module * node_proto = nodes.back().prototype;

    //     Roulette roulette;
    //     size_t total_c_links = 0;
    //     for (auto & chain_map_it : node_proto->chains()) {
    //         const Module::Chain & chain = chain_map_it.second;
    //         for (auto & c_link : chain.c_links) {
    //             Transform tx = c_link.tx * base_tx;

    //             if (!nodes.collides(
    //                         tx.collapsed(),
    //                         node_proto->radius_)) {
    //                 compat_links.push_back(&c_link);
    //                 const float clc = node_proto->c_link_count();
    //                 roulette.cmlprobs.push_back(total_c_links);
    //                 total_c_links += clc;
    //             }
    //         }
    //     }

    //     if (compat_links.empty())
    //         break;

    //     roulette.normalize(total_c_links);

    //     // Pick a random valid neighbour
    //     const Module::Link * link = roulette.rand_item(compat_links);

    //     // Grow shape
    //     nodes.emplace_back(link->mod, link->tx * base_tx);
    // }
}

void NodeList::destructive_randomize() {
    this->clear();
    randomize(this->end(), TerminusType::ANY, NodeList::MAX_LEN);
}

void NodeList::auto_mutate() {
    // Try point mutate first, if not possible then
    // do limb mutate. If still not possible, create
    // a new chromosome

    if (!point_mutate()) {
        if (!limb_mutate()) {
            destructive_randomize();
        }
    }
}

IdPairs get_crossing_ids(
    const NodeList & mother,
    const NodeList & father) {
    // Compute index pairs of mother and father nodes at which cross mutation
    // is possible
    IdPairs crossing_ids;
    const size_t mn_len = mother.size();;
    const size_t fn_len = father.size();

    for (long i = 1; i < (long) mn_len - 1; i++) {
        // Using i as nodes1 left limb cutoff
        for (long j = 1; j < (long) fn_len - 1; j++) {
            if (mother.at(i).prototype == father.at(j).prototype) {
                crossing_ids.push_back(IdPair(i, j));
            }
        }
    }

    return crossing_ids;
}

bool NodeList::cross_mutate(const NodeList & father) {
    bool cross_ok = false;

#ifndef NO_CROSS_MUTATE
    // Current chromosome is mother
    IdPairs crossing_ids = get_crossing_ids(*this, father);

    size_t tries = 0;
    // cross can fail - if resultant nodes collide during synth
    while (tries < MAX_FREECANDIDATE_MUTATE_FAILS and
            crossing_ids.size()) {
        // Pick random crossing point
        const IdPair & cross_point = pick_random(crossing_ids);

        NodeList new_nodes;

        new_nodes.insert(
            new_nodes.end(),
            this->begin(),
            this->begin() + cross_point.x);

        new_nodes.insert(
            new_nodes.end(),
            father.begin() + cross_point.y,
            father.end());

        die("Collision check here?\n");
        if (synthesise(new_nodes)) {
            *this = new_nodes;
            cross_ok = true;
            break;
        }

        tries++;
    }
#endif // NO_CROSS_MUTATE

    return cross_ok;
}

enum PointMutateMode {
    SwapMode,
    InsertMode,
    DeleteMode,
    EnumSize
};

/*
 * Point Mutation tries the following modifications:
 *   1. Swap with another node
 *   2. Insert a node
 *   3. Delete the node
 *
 * The selection is uniform probability without replacement.
 */
bool NodeList::point_mutate() {
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
                            NodeList test_nodes(nodes_);
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
                        NodeList test_nodes(nodes_);
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
                        NodeList test_nodes(
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
            panic_if(pm_mode < 0 or pm_mode >= PointMutateMode::EnumSize,
                     "Invalid pm_mode in Chromosome::pointMutate()\n");
        }
        } // end of pm_mode switch
    }
#endif // NO_POINT_MUTATE

    return mutate_success;
}

bool NodeList::limb_mutate() {
    bool mutate_success = false;

#ifndef NO_LIMB_MUTATE
    // Pick a node that can host an alternative limb
    size_t sever_id = 0;
    if (pick_sever_point(sever_id)) {
        // Server the limb
        if (mutate_left_limb)
            nodes_.erase(nodes_.begin(), nodes_.begin() + sever_id);
        else
            nodes_.erase(nodes_.begin() + sever_id + 1, nodes_.end());

        // Re-generate that whole limb
        for (size_t i = 0; i < MAX_FREECANDIDATE_MUTATE_FAILS; i++) {
            if (mutate_left_limb)
                gen_random_nodes_reverse(nodes_);
            else
                gen_random_nodes(nodes_);
        }

        mutate_success = true;
    }
#endif // NO_LIMB_MUTATE

    return mutate_success;
}

}  /* elfin */