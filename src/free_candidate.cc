#include "free_candidate.h"

#include <sstream>
#include <exception>

#include "random_utils.h"
#include "kabsch.h"
#include "input_manager.h"
#include "id_types.h"

#ifndef NDEBUG
#include "debug_utils.h"
#endif  /* ifndef NDEBUG */

#define NO_POINT_MUTATE
#define NO_LIMB_MUTATE
#define NO_CROSS_MUTATE

#if defined(NO_POINT_MUTATE) || \
    defined(NO_LIMB_MUTATE) || \
    defined(NO_CROSS_MUTATE)
#warning "Mutation code not being compiled!!"
#endif

namespace elfin {

/* private */

/*
 * Compute index pairs of mother and father nodes at which cross mutation is
 * possible.
 */
IdPairs get_crossing_ids(
    const Nodes & mother,
    const Nodes & father) {
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

bool FreeCandidate::cross_mutate(
    const Candidate * mother,
    const Candidate * father) {
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

bool FreeCandidate::point_mutate() {
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
            panic_if(pm_mode < 0 or pm_mode >= PointMutateMode::EnumSize,
                     "Invalid pm_mode in Chromosome::pointMutate()\n");
        }
        } // end of pm_mode switch
    }
#endif // NO_POINT_MUTATE

    return mutate_success;
}

/*
 * Finds points at which a severance would allow a different limb to be grown.
 * (Modifies "id_out")
 */
bool FreeCandidate::pick_sever_point(
    size_t & id_out,
    const TerminusType term) {

    bool success = false;
    std::vector<size_t> non_dead_end_ids;

    for (size_t i = 0; i < nodes_.size(); ++i) {
        const Module * proto = nodes_.at(i)->prototype();
        if ((term == N and proto->counts.n_link > 1) or
                (term == C and proto->counts.c_link > 1) or
                (term == ANY and proto->counts.all_link() > 2)) {

            non_dead_end_ids.push_back(i);

        }
    }

    if (!non_dead_end_ids.empty()) {
        id_out = pick_random(non_dead_end_ids);
        success = true;
    }

    return success;
}

bool FreeCandidate::limb_mutate() {
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

void FreeCandidate::grow(const size_t tip_index, TerminusType term) {
#ifndef NDEBUG
    // cry if nodes_ does not at least have the tip_index
    DEBUG(tip_index >= nodes_.size());
#endif  /* ifndef NDEBUG */

    Node * tip_node = nodes_.at(tip_index);

    if (term == TerminusType::ANY) {
        term = random_term();
    }

    // cry if no free term available
    while (nodes_.size() < Candidate::MAX_LEN) {
#ifndef NDEBUG
        // pick random chain and then random link
        DEBUG(tip_node->term_tracker().get_free_size(term) == 0);
#endif  /* ifndef NDEBUG */

        auto & free_chain_ids = tip_node->term_tracker().get_free(term);
        wrn("free size: %lu\n", free_chain_ids.size());
        const size_t tip_chain_id = pick_random(free_chain_ids);
        wrn("tip_chain_id: %lu\n", tip_chain_id);
        auto & chain = tip_node->prototype()->chains.at(tip_chain_id);
        auto & links = chain.get_links(term);
        const auto & link = pick_random(links);
        wrn("link: %x\n", &link);

        Node * new_node = new Node(link.mod, tip_node->tx() * link.tx);
        Node::connect(tip_node, tip_chain_id, term, new_node, link.target_chain_id);
        nodes_.push_back(new_node);

        tip_node = new_node;

#ifndef NDEBUG
        DEBUG(0 == new_node->term_tracker().get_free_size(TerminusType::ANY));
#endif  /* ifndef NDEBUG */

        if (new_node->term_tracker().get_free_size(TerminusType::N) == 0) {
            term = TerminusType::C;
        }
        else if (new_node->term_tracker().get_free_size(TerminusType::C) == 0) {
            term = TerminusType::N;
        }
        else {
            wrn("Shouldn't reach here because we're only using basic modules!\n");
            die("Node: %s\n", new_node->to_string().c_str());
            term = random_term();
        }
    }
}

void FreeCandidate::regrow() {
    nodes_.clear();

    // Pick random starting node (basic module)
    const Module * mod = XDB.get_drawables().basic.all.rand_item();

    Node * new_node = new Node(mod);
    nodes_.push_back(new_node);

    // Pick random terminus
    TerminusType term;
    if (new_node->term_tracker().get_free_size(TerminusType::N) == 0) {
        term = TerminusType::C;
    }
    else if (new_node->term_tracker().get_free_size(TerminusType::C) == 0) {
        term = TerminusType::N;
    }
    else {
        term = TerminusType::ANY;
    }

    grow(0, term);
}

/* public */
std::string FreeCandidate::to_string() const {
    std::stringstream ss;

    ss << "FreeCandidate " << this << ":\n";
    ss << Candidate::to_string();

    return ss.str();
}

void FreeCandidate::score(const WorkArea & wa) {
    V3fList points;
    for (auto & n : nodes_) {
        points.emplace_back(n->tx().collapsed());
    }
    score_ = kabsch_score(points, wa);
}

FreeCandidate * FreeCandidate::clone() const {
    return new FreeCandidate(*this);
}

}  /* elfin */