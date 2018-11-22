#include "free_candidate.h"

#include <sstream>
#include <exception>

#include "random_utils.h"
#include "kabsch.h"
#include "input_manager.h"

namespace elfin {

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
        // rank -1 is code for destructive randomize
        nodes_.destructive_randomize();
    }
    else if (rank <= CUTOFFS.survivors) {
        *this = *((FreeCandidate *) candidates.at(rank));
    }
    else {
        // Replicate mother
        const size_t mother_id = get_dice(CUTOFFS.survivors);
        const FreeCandidate * mother =
            (FreeCandidate*) candidates.at(mother_id);
        *this = *mother;

        const size_t mutation_dice =
            CUTOFFS.survivors +
            get_dice(CUTOFFS.non_survivors);
        if (mutation_dice <= CUTOFFS.cross) {
            // Pick father
            const size_t father_id = get_dice(CUTOFFS.pop_size);
            const FreeCandidate * father =
                (FreeCandidate*) candidates.at(father_id);

            // Fall back to auto mutate if cross fails
            if (!nodes_.cross_mutate(father->nodes())) {
                // Pick a random parent to inherit from and then mutate
                nodes_.auto_mutate();
                mt_counters.cross_fail++;
            }

            mt_counters.cross++;
        }
        else if (mutation_dice <= CUTOFFS.point) {
            if (!nodes_.point_mutate()) {
                nodes_.destructive_randomize();
                mt_counters.point_fail++;
            }
            mt_counters.point++;
        }
        else if (mutation_dice <= CUTOFFS.limb) {
            if (!nodes_.limb_mutate()) {
                nodes_.destructive_randomize();
                mt_counters.limb_fail++;
            }
            mt_counters.limb++;
        }
        else {
            // Individuals not covered by specified mutation
            // rates undergo random destructive mutation
            nodes_.destructive_randomize();
            mt_counters.rand++;
        }
    }
}

/* public */

FreeCandidate * FreeCandidate::clone() const {
    return new FreeCandidate(*this);
}

}  /* elfin */