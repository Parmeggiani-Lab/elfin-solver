#include "free_candidate.h"

#include <sstream>

namespace elfin {

/* strings */
std::string FreeCandidate::to_string(const IdNameMap & inm) const {
    std::stringstream ss;

    ss << "FreeCandidate " << this << ":\n";
    ss << Candidate::to_string(inm);

    return ss.str();
}

void FreeCandidate::init(const WorkArea & wa) {

}

void FreeCandidate::score(const WorkArea & wa) {

}

void FreeCandidate::mutate(
    size_t rank, 
    const MutationCutoffs & mt_cutoffs,
    const MutationCounters & mt_counts) {
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


}  /* elfin */