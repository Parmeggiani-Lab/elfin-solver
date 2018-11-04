#include "free_candidate.h"

#include <sstream>

#include "roulette.h"
#include "pair_relationship.h"
#include "parallel_utils.h"
#include "math_utils.h"
#include "kabsch.h"
#include "counter_structs.h"
#include "options.h"

namespace elfin {

/* private */

Nodes FreeCandidate::gen_random_nodes(size_t max_len, Nodes & nodes) {
    const size_t dim = REL_MAT.size();

    // A roulette wheel represents the probability of
    // each node being picked as the next node, based
    // on the number of neighbours they have.
    //
    // This is so as to not pick "dead-end" nodes
    // often, which can result in very "boring" shapes
    // e.g. formed by repetition of just one node
    //
    // Once a dead-end node is picked, further nodes
    // are simply repeated and we don't want this to
    // happen often.

    if (nodes.size() == 0) {
        // Pick random starting node
        const size_t first_node_id = CTERM_ROULETTE.at(get_dice(CTERM_ROULETTE.size()));
        nodes.emplace_back(first_node_id, 0, 0, 0);
    }
    else {
        synthesise(nodes);
    }

    while (nodes.size() < max_len) {
        std::vector<size_t> roulette_wheel;
        const Node & curr_node = nodes.back();

        // Compute whether each neighbour is colliding
        const std::vector<PairRelationship *> rr = REL_MAT.at(curr_node.id);
        for (size_t i = 0; i < dim; i++) {
            const PairRelationship * pr_ptr = rr.at(i);

            // Create roulette based on number of RHS neighbours
            // of the current neighbour being considered
            if (pr_ptr and nodes.size() >= 2 and !collides(i,
                                    pr_ptr->com_b_,
                                    nodes.begin(),
                                    nodes.end() - 2)) {
                for (int j = 0; j < LINK_COUNTS.at(i).y; j++)
                    roulette_wheel.push_back(i);
            }
        }

        if (roulette_wheel.size() == 0)
            break;

        // Pick a random valid neighbour
        const uint next_node_id = roulette_wheel.at(get_dice(roulette_wheel.size()));

        const PairRelationship * next_node_pr = rr.at(next_node_id);

        // Grow shape
        for (auto & n : nodes) {
            n.com = n.com.dot(next_node_pr->rot_);
            n.com += next_node_pr->tran_;
        }

        nodes.emplace_back(next_node_id, 0, 0, 0);
    }

    return nodes;
}

Nodes FreeCandidate::gen_random_nodes_reverse(size_t max_len, Nodes & nodes) {
    const size_t dim = REL_MAT.size();

    if (nodes.size() == 0) {
        // Pick random starting node
        const size_t first_node_id = CTERM_ROULETTE.at(get_dice(CTERM_ROULETTE.size()));
        nodes.emplace_back(first_node_id, 0, 0, 0);
    }
    else {
        synthesise_reverse(nodes);
    }

    // Reverse order so growth tip is at back
    std::reverse(nodes.begin(), nodes.end());

    while (nodes.size() < max_len) {
        std::vector<uint> roulette_wheel;
        const Node & curr_node = nodes.back();

        // Compute whether each neighbour is colliding
        for (size_t i = 0; i < dim; i++) {
            const PairRelationship * pr_ptr = REL_MAT.at(i).at(curr_node.id);
            if (pr_ptr == NULL)
                continue;

            const Point3f checkpoint = pr_ptr->tran_;

            // Create roulette based on number of LHS neighbours
            // of the current neighbour being considered
            if (!collides(i,
                          checkpoint,
                          nodes.begin(),
                          nodes.end() - 2)) {
                for (int j = 0; j < LINK_COUNTS.at(i).x; j++)
                    roulette_wheel.push_back(i);
            }
        }

        if (roulette_wheel.size() == 0)
            break;

        // Pick a random valid neighbour
        const uint next_node_id =
            roulette_wheel.at(get_dice(roulette_wheel.size()));

        const PairRelationship * next_node_pr =
            REL_MAT.at(next_node_id).at(curr_node.id);

        // Grow shape
        for (auto & n : nodes) {
            n.com -= next_node_pr->tran_;
            n.com = n.com.dot(next_node_pr->rot_inv_);
        }

        nodes.emplace_back(next_node_id, 0, 0, 0);
    }

    // Reverse the reverse!
    std::reverse(nodes.begin(), nodes.end());

    return nodes;
}

bool FreeCandidate::synthesise_reverse(Nodes & nodes) {
    const uint N = nodes.size();
    if (N == 0)
        return true;

    // Zero all coords so they don't interfere with
    // collision check before being synth'd
    for (auto & n : nodes)
        n.com.x = (n.com.y = (n.com.z = 0));

    for (int i = N - 1; i > 0; i--) {
        const auto & lhs_node = nodes.at(i - 1);
        const auto & rhs_node = nodes.at(i);

        // Check collision
        const PairRelationship * new_node_pr =
            REL_MAT.at(lhs_node.id).at(rhs_node.id);

        if (new_node_pr == NULL) {
            // Fatal failure; diagnose!
            err("Synthesise(): impossible pair! %d(%s) <-x-> %d(%s)\n",
                lhs_node.id,
                ID_NAME_MAP.at(lhs_node.id).c_str(),
                rhs_node.id,
                ID_NAME_MAP.at(rhs_node.id).c_str());

            die("Fatal error in synthesise_reverse(): should never use impossible pair\n");
        }

        const Point3f checkpoint = new_node_pr->tran_;

        if (collides(lhs_node.id,
                     checkpoint,
                     nodes.begin() + i + 2,
                     nodes.end()))
            return false;

        // Grow shape
        for (int j = N - 1; j > i - 1; j--) {
            auto & n = nodes.at(j);
            n.com -= new_node_pr->tran_;
            n.com = n.com.dot(new_node_pr->rot_inv_);
        }
    }

    return true;
}

bool FreeCandidate::synthesise(Nodes & nodes) {
    if (nodes.size() == 0)
        return true;

    // Zero all coords so they don't interfere with
    // collision check before being synth'd
    for (auto & n : nodes)
        n.com.x = (n.com.y = (n.com.z = 0));

    for (int i = 1; i < nodes.size(); i++) {
        const auto & lhs_node = nodes.at(i - 1);
        const auto & rhs_node = nodes.at(i);

        // Check collision
        const PairRelationship * new_node_pr =
            REL_MAT.at(lhs_node.id).at(rhs_node.id);

        if (new_node_pr == NULL) {
            // Fatal failure; diagnose!
            err("Synthesise(): impossible pair! %d(%s) <-x-> %d(%s)\n",
                lhs_node.id,
                ID_NAME_MAP.at(lhs_node.id).c_str(),
                rhs_node.id,
                ID_NAME_MAP.at(rhs_node.id).c_str());

            die("Fatal error in synthesise(): should never use impossible pair\n");
        }

        if (nodes.size() < 2 or
            collides(rhs_node.id,
                     new_node_pr->com_b_,
                     nodes.begin(),
                     nodes.begin() + i - 2))
            return false;

        // Grow shape
        for (int j = 0; j < i; j++) {
            auto & n = nodes.at(j);
            n.com = n.com.dot(new_node_pr->rot_);
            n.com += new_node_pr->tran_;
        }
    }

    return true;
}

void FreeCandidate::randomize() {
    nodes_ = gen_random_nodes(CANDIDATE_LENGTHS.max, nodes_);
}

void FreeCandidate::auto_mutate() {
    // Try point mutate first, if not possible then
    // do limb mutate. If still not possible, create
    // a new chromosome

    if (!this->point_mutate()) {
        if (!this->limb_mutate()) {
            this->randomize();
        }
    }
}

bool FreeCandidate::cross_mutate(
    const FreeCandidate & father,
    FreeCandidate & out) const {
    // Current chromosome is mother
    IdPairs crossing_ids;

    const Nodes & father_nodes = father.nodes();
    const ulong fn_len = father_nodes.size();
    const ulong mn_len = nodes_.size();

    // In below comments nodes1 = this, nodes2 = other
    for (size_t i = 0; i < mn_len; i++) {
        // Using i as nodes1 left limb cutoff
        const ulong left_limb_len = i + 1; // This includes the node i
        const ulong max_j = std::min(
                                std::max(
                                    fn_len - (CANDIDATE_LENGTHS.min - left_limb_len) - 1, // -1 to account for the duplicate cross point node
                                    (ulong) 0),
                                fn_len - 1);
        const ulong min_j = std::min(
                                std::max(
                                    fn_len - (CANDIDATE_LENGTHS.max - left_limb_len) - 1, // -1 to account for the duplicate cross point node
                                    (ulong) 0),
                                fn_len - 1);
        for (int j = min_j; j < max_j; j++) {
            if (nodes_.at(i).id == father_nodes.at(j).id) {
                crossing_ids.push_back(IdPair(i, j));
            }
        }
    }

    if (crossing_ids.size() > 0) {
        // cross can fail - if resultant nodes collide during synth
        for (size_t i = 0; i < MAX_FREECANDIDATE_MUTATE_FAILS; i++) {
            // Pick random crossing point
            const IdPair & cross_point = crossing_ids.at(get_dice(crossing_ids.size()));
            const uint mother_node_id = cross_point.x;
            const uint father_node_id = cross_point.y;

            const Nodes & father_nodes = father.nodes();

            Nodes new_nodes;
            new_nodes.insert(
                new_nodes.end(),
                nodes_.begin(),
                nodes_.begin() + mother_node_id);
            new_nodes.insert(
                new_nodes.end(),
                father_nodes.begin() + father_node_id,
                father_nodes.end());

            if (synthesise(new_nodes)) {
                out = FreeCandidate(new_nodes);
                return true;
            }
        }
    }

    return false;
}

enum PointMutateMode {
    SwapMode,
    InsertMode,
    DeleteMode,
    EnumSize
};

bool FreeCandidate::point_mutate() {
    // There are 3 ways for point mutate:
    // 1. Swap with another node
    // 2. Insert a node
    // 3. Delete the node
    // As of now it uses equal probability.
    // Could be opened up as a setting.
    const size_t dim = REL_MAT.size();
    const size_t n_nodes = nodes_.size();
    std::vector<PointMutateMode> modes =
    {
        PointMutateMode::SwapMode,
        PointMutateMode::InsertMode,
        PointMutateMode::DeleteMode
    };

    while (modes.size() > 0) {
        // Draw a random mode without replacement
        const int mode_index = get_dice(modes.size());
        const PointMutateMode pm_mode = modes.at(mode_index);
        modes.erase(modes.begin() + mode_index);

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
                for (int j = 0; j < dim; j++) {
                    // Make sure it's not the original one
                    if (j != nodes_.at(i).id) {
                        // Check whether i can be exchanged for j
                        if (
                            (i < n_nodes) // i can be n_nodes, which is used for insertion check
                            and
                            (i == 0 or // Pass if i is the left end
                             REL_MAT.at(nodes_.at(i - 1).id).at(j) != NULL)
                            and
                            (i == n_nodes - 1 or // Pass if i is the right end
                             REL_MAT.at(j).at(nodes_.at(i + 1).id) != NULL)
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

            // Pick a random one, or fall through to next case
            if (swappable_ids.size() > 0) {
                const IdPair & ids = swappable_ids.at(get_dice(swappable_ids.size()));
                nodes_.at(ids.x).id = ids.y;

                synthesise(nodes_); // This is guaranteed to succeed
                return true;
            }
        }

        case PointMutateMode::InsertMode: {
            IdPairs insertable_ids;
            if (n_nodes < CANDIDATE_LENGTHS.max) {
                for (size_t i = 0; i < n_nodes; i++) {
                    for (int j = 0; j < dim; j++) {
                        // Check whether j can be inserted before i
                        if (
                            (i == 0 or // Pass if inserting at the left end
                             REL_MAT.at(nodes_.at(i - 1).id).at(j) != NULL)
                            and
                            (i == n_nodes or // Pass if appending at the right end
                             REL_MAT.at(j).at(nodes_.at(i).id) != NULL)
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
                    const IdPair & ids = insertable_ids.at(get_dice(insertable_ids.size()));
                    Node new_node;
                    new_node.id = ids.y;
                    nodes_.insert(nodes_.begin() + ids.x, //This is insertion before i
                                  new_node);

                    synthesise(nodes_); // This is guaranteed to succeed
                    return true;
                }
            }
        }

        case PointMutateMode::DeleteMode: {
            std::vector<size_t> deletable_ids;
            if (n_nodes > CANDIDATE_LENGTHS.min) {
                for (size_t i = 0; i < n_nodes; i++) {
                    // Check whether i can be deleted
                    if (
                        (i < n_nodes) // i can be n_nodes, which is used for insertion check
                        and
                        (i == 0 or i == n_nodes - 1 or // Pass if i is at either end
                         REL_MAT.at(nodes_.at(i - 1).id).at(nodes_.at(i + 1).id) != NULL)
                    ) {
                        // Make sure resultant shape won't collide with itself
                        Nodes test_nodes(nodes_);
                        test_nodes.erase(test_nodes.begin() + i);

                        // dbg("checking deletion at %d/%d of %s\n",
                        //     i, n_nodes, to_string().c_str());
                        if (synthesise(test_nodes)) deletable_ids.push_back(i);
                    }
                }

                // Pick a random one, or report impossible
                if (deletable_ids.size() > 0) {
                    nodes_.erase(nodes_.begin() + deletable_ids.at(get_dice(deletable_ids.size())));

                    synthesise(nodes_); // This is guaranteed to succeed
                    return true;
                }
            }
        }

        default: {
            // Fell through all cases without mutating
            // Do nothing unless pm_mode is strange
            panic_if(pm_mode < 0 or pm_mode >= PointMutateMode::EnumSize,
                     "Invalid pm_mode in Chromosome::pointMutate()\n");
        }
        } // end of pm_mode switch
    }

    return false;
}

bool FreeCandidate::limb_mutate() {
    const size_t N = nodes_.size();
    if(N < 2)
        return false;

    // Pick a node that can host an alternative limb
    uint sever_id = -1;
    bool mutate_left_limb = false;

    for (size_t i = 0; i < MAX_FREECANDIDATE_MUTATE_FAILS; i++) {
        const uint node_index = get_dice(N - 1) + 1;
        const uint node_id = nodes_.at(node_index).id;

        const IdPair & ids = LINK_COUNTS.at(node_id);

        if (ids.x == 1 and ids.y == 1)
            continue;

        if (ids.x == 1)
            mutate_left_limb = false;
        else if (ids.y == 1)
            mutate_left_limb = true;
        else
            mutate_left_limb = get_dice(2);

        sever_id = node_index;
        break;
    }

    if (sever_id == -1)
        return false;

    // Server the limb
    if (mutate_left_limb)
        nodes_.erase(nodes_.begin(), nodes_.begin() + sever_id);
    else
        nodes_.erase(nodes_.begin() + sever_id + 1, nodes_.end());

    // Re-generate that whole limb
    Nodes new_nodes;
    for (size_t i = 0; i < MAX_FREECANDIDATE_MUTATE_FAILS; i++) {
        new_nodes = mutate_left_limb ?
                    gen_random_nodes_reverse(CANDIDATE_LENGTHS.max, nodes_) :
                    gen_random_nodes(CANDIDATE_LENGTHS.max, nodes_);

        if (new_nodes.size() >= CANDIDATE_LENGTHS.min)
            break;
    }

    if (new_nodes.size() < CANDIDATE_LENGTHS.min)
        return false;

    nodes_ = new_nodes;
    return true;
}


/* public */

std::string FreeCandidate::to_string() const {
    std::stringstream ss;

    ss << "FreeCandidate " << this << ":\n";
    ss << Candidate::to_string();

    return ss.str();
}

void FreeCandidate::score(const WorkArea & wa) {
    score_ = kabsch_score(nodes_, wa);
}

void FreeCandidate::mutate(
    long rank,
    MutationCounters & mt_counters,
    const Candidates & candidates) {

    if (rank == -1) {
        this->randomize();
    }
    else if (rank < POPULATION_COUNTERS.survivors) {
        *this = *((FreeCandidate *) candidates.at(rank));
    }
    else {
        const size_t evolution_dice =
            POPULATION_COUNTERS.survivors +
            get_dice(POPULATION_COUNTERS.non_survivors);

        if (evolution_dice < MUTATION_CUTOFFS.cross) {
            long motherId = get_dice(get_dice(2) ?
                                     POPULATION_COUNTERS.survivors :
                                     POPULATION_COUNTERS.pop_size);
            long fatherId = get_dice(get_dice(2) ?
                                     POPULATION_COUNTERS.survivors :
                                     POPULATION_COUNTERS.pop_size);

            const FreeCandidate * mother = (FreeCandidate*) candidates.at(motherId);
            const FreeCandidate * father = (FreeCandidate*) candidates.at(fatherId);

            // Check compatibility
            if (!mother->cross_mutate(*father, *this)) {
                // Pick a random parent to inherit from and then mutate
                *this = *mother;
                this->auto_mutate();
                mt_counters.cross_fail++;
            }
            mt_counters.cross++;
        }
        else {
            // Replicate a surviver
            const ulong parent_id = get_dice(POPULATION_COUNTERS.survivors);
            *this = *((FreeCandidate *) candidates.at(parent_id));

            if (evolution_dice < MUTATION_CUTOFFS.point) {
                if (!this->point_mutate()) {
                    this->randomize();
                    mt_counters.point_fail++;
                }
                mt_counters.point++;
            }
            else if (evolution_dice < MUTATION_CUTOFFS.limb) {
                if (!this->limb_mutate()) {
                    this->randomize();
                    mt_counters.limb_fail++;
                }
                mt_counters.limb++;
            }
            else {
                // Individuals not covered by specified mutation
                // rates undergo random destructive mutation
                this->randomize();
                mt_counters.rand++;
            }
        }
    }
}

Candidate * FreeCandidate::new_copy() const {
    Candidate * nc = new FreeCandidate(*this);
    return nc;
}

}  /* elfin */