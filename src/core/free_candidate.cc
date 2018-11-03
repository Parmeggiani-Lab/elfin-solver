#include "free_candidate.h"

#include <sstream>

#include "roulette.h"
#include "pair_relationship.h"
#include "parallel_utils.h"
#include "math_utils.h"

namespace elfin {

/* public */

std::string FreeCandidate::to_string() const {
    std::stringstream ss;

    ss << "FreeCandidate " << this << ":\n";
    ss << Candidate::to_string();

    return ss.str();
}

void FreeCandidate::score(const WorkArea & wa) {

}

void FreeCandidate::mutate(
    long rank,
    const CandidateLengths & cd_lens,
    const MutationCounters & mt_counters) {
    if (rank == -1) // code for randomize
    {
        nodes_ = gen_random_nodes(cd_lens.max);
    }
    else
    {
        // Chromosome & chromo_to_evolve = buff_pop_data_[i];
        // const ulong evolution_dice = counts_.survivors +
        //                              get_dice_funct(counts_.non_survivors);

        // if (evolution_dice < cross_cutoff_)
        // {
        //     long motherId, fatherId;
        //     if (get_dice_funct(2))
        //     {
        //         motherId = get_dice_funct(counts_.survivors);
        //         fatherId = get_dice_funct(counts_.pop_size);
        //     }
        //     else
        //     {
        //         motherId = get_dice_funct(counts_.pop_size);
        //         fatherId = get_dice_funct(counts_.survivors);
        //     }

        //     const Chromosome & mother = curr_pop_data_[motherId];
        //     const Chromosome & father = curr_pop_data_[fatherId];

        //     // Check compatibility
        //     if (!(mother.*cross_chromosome_funct)(father, chromo_to_evolve))
        //     {
        //         // Pick a random parent to inherit from and then mutate
        //         (chromo_to_evolve.*assign_chromo_funct)(mother);
        //         (chromo_to_evolve.*auto_mutate_chromo_funct)();
        //         mt_counts.cross_fail++;
        //     }
        //     mt_counts.cross++;
        // }
        // else
        // {
        //     // Replicate a high ranking parent
        //     const ulong parentId = get_dice_funct(counts_.survivors);
        //     (chromo_to_evolve.*assign_chromo_funct)(curr_pop_data_[parentId]);

        //     if (evolution_dice < point_mutate_cutoff_)
        //     {
        //         if (!(chromo_to_evolve.*point_mutate_chromo_funct)())
        //             (chromo_to_evolve.*randomise_chromo_funct)();
        //         mt_counts.point++;
        //     }
        //     else if (evolution_dice < limb_mutate_cutoff_)
        //     {
        //         if (!(chromo_to_evolve.*limb_mutate_chromo_funct)())
        //             (chromo_to_evolve.*randomise_chromo_funct)();
        //         mt_counts.limb++;
        //     }
        //     else
        //     {
        //         // Individuals not covered by specified mutation
        //         // rates undergo random destructive mutation
        //         (chromo_to_evolve.*randomise_chromo_funct)();
        //         mt_counts.rand++;
        //     }
        // }
    }
}

Candidate * FreeCandidate::new_copy() const {
    Candidate * nc = new FreeCandidate(*this);
    return nc;
}

std::vector<Candidate::Node> FreeCandidate::gen_random_nodes(int gen_max_len) const {
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

    std::vector<Candidate::Node> nodes;
    if (nodes.size() == 0)
    {
        // Pick random starting node
        const size_t first_node_id = CTERM_ROULETTE.at(get_dice(CTERM_ROULETTE.size()));
        nodes.emplace_back(first_node_id, 0, 0, 0);
    }
    else
    {
        // synthesise(nodes);
    }

    while (nodes.size() <= gen_max_len)
    {
        std::vector<size_t> roulette_wheel;
        const Node & curr_node = nodes.back();

        // Compute whether each neighbour is colliding
        const std::vector<PairRelationship *> rr = REL_MAT.at(curr_node.id);
        for (int i = 0; i < dim; i++)
        {
            const PairRelationship * pr_ptr = rr.at(i);

            // Create roulette based on number of RHS neighbours
            // of the current neighbour being considered
            if (pr_ptr && !collides(i,
                                   pr_ptr->com_b_,
                                   nodes.begin(),
                                   nodes.end() - 2))
            {
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
        for (auto & n : nodes)
        {
            n.com = n.com.dot(next_node_pr->rot_);
            n.com += next_node_pr->tran_;
        }

        nodes.emplace_back(next_node_id, 0, 0, 0);
    }

    return nodes;
}

}  /* elfin */