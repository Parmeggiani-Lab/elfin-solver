#include "free_candidate.h"

#include <sstream>
#include <exception>

#include "database.h"
#include "roulette.h"
#include "random_utils.h"
#include "parallel_utils.h"
#include "kabsch.h"
#include "counter_structs.h"
#include "options.h"
#include "terminus_type.h"
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

/* private */

void FreeCandidate::randomize() {
    nodes_.randomize(nodes_.begin(), TerminusType::ANY, NodeList::MAX_LEN);
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

IdPairs get_crossing_ids(
    const FreeCandidate & mother,
    const FreeCandidate & father) {
    // Compute index pairs of mother and father nodes at which cross mutation
    // is possible
    IdPairs crossing_ids;
    const NodeList & mother_nodes = mother.nodes();
    const size_t mn_len = mother_nodes.size();
    const NodeList & father_nodes = father.nodes();
    const size_t fn_len = father_nodes.size();

    for (long i = 1; i < (long) mn_len - 1; i++) {
        // Using i as nodes1 left limb cutoff
        for (long j = 1; j < (long) fn_len - 1; j++) {
            if (mother_nodes.at(i).prototype == father_nodes.at(j).prototype) {
                crossing_ids.push_back(IdPair(i, j));
            }
        }
    }

    return crossing_ids;
}

bool FreeCandidate::cross_mutate(const FreeCandidate & father) {
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
        const NodeList & father_nodes = father.nodes();

        NodeList new_nodes;

        new_nodes.insert(
            new_nodes.end(),
            nodes_.begin(),
            nodes_.begin() + cross_point.x);

        new_nodes.insert(
            new_nodes.end(),
            father_nodes.begin() + cross_point.y,
            father_nodes.end());

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

bool FreeCandidate::limb_mutate() {
    bool mutate_success = false;

#ifndef NO_LIMB_MUTATE
    // Pick a node that can host an alternative limb
    size_t sever_id = 0;
    if (pick_sever_point(nodes_, sever_id)) {
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
    const CandidateList & candidates) {

    if (rank == -1) {
        // rank -1 is code for randomize
        this->randomize();
    }
    else if (rank <= POPULATION_COUNTERS.survivors) {
        *this = *((FreeCandidate *) candidates.at(rank));
    }
    else {
        // Replicate mother
        const size_t mother_id = get_dice(POPULATION_COUNTERS.survivors);
        const FreeCandidate * mother =
            (FreeCandidate*) candidates.at(mother_id);
        *this = *mother;

        const size_t mutation_dice =
            POPULATION_COUNTERS.survivors +
            get_dice(POPULATION_COUNTERS.non_survivors);
        if (mutation_dice <= MUTATION_CUTOFFS.cross) {
            // Pick father
            const size_t father_id = get_dice(POPULATION_COUNTERS.pop_size);
            const FreeCandidate * father =
                (FreeCandidate*) candidates.at(father_id);

            // Fall back to auto mutate if cross fails
            if (!this->cross_mutate(*father)) {
                // Pick a random parent to inherit from and then mutate
                this->auto_mutate();
                mt_counters.cross_fail++;
            }

            mt_counters.cross++;
        }
        else if (mutation_dice <= MUTATION_CUTOFFS.point) {
            if (!this->point_mutate()) {
                this->randomize();
                mt_counters.point_fail++;
            }
            mt_counters.point++;
        }
        else if (mutation_dice <= MUTATION_CUTOFFS.limb) {
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

/* public */

FreeCandidate * FreeCandidate::clone() const {
    return new FreeCandidate(*this);
}

}  /* elfin */