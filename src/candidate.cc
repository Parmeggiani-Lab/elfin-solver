#include "candidate.h"

#include <sstream>

#include "random_utils.h"

namespace elfin {

/* static data members */
size_t Candidate::MAX_LEN_ = 0;
const size_t & Candidate::MAX_LEN = Candidate::MAX_LEN_;

/* protected */

/*
 * Checks whether new_com is too close to any other com.
 */
bool Candidate::collides(
    const Vector3f & new_com,
    const float mod_radius) const {

    for (const auto node : nodes_) {
        const float sq_com_dist = node->tx().collapsed().sq_dist_to(new_com);
        const float required_com_dist = mod_radius +
                                        node->prototype()->radius_;
        if (sq_com_dist < (required_com_dist * required_com_dist)) {
            return true;
        }
    }

    return false;
}

/*
 * Try point mutate first, if not possible then do limb mutate. If still not
 * possible, create a new chromosome.
 */
void Candidate::auto_mutate() {
    if (!point_mutate()) {
        if (!limb_mutate()) {
            regrow();
        }
    }
}

/* public */

StrList Candidate::get_node_names() const {
    StrList res;

    for (auto & n : nodes_) {
        res.emplace_back(n->prototype()->name_);
    }

    return res;
}

std::string Candidate::to_string() const {
    std::stringstream ss;

    const size_t N = nodes_.size();
    for (size_t i = 0; i < N; ++i)
    {
        ss << "Nodes[#" << (i + 1) << " / " << N << "]: ";
        ss << nodes_[i]->to_string() << std::endl;
    }

    return ss.str();
}

std::string Candidate::to_csv_string() const {
    std::stringstream ss;

    for (auto & n : nodes_) {
        ss << n->to_csv_string() << std::endl;
    }

    return ss.str();
}

Crc32 Candidate::checksum() const
{
    // Compute checksum lazily because it's only used once per generation
    Crc32 crc = 0xffff;
    for (auto & n : nodes_) {
        // Compute checksum based on prototype identity sequence
        const Module * prot = n->prototype();
        checksum_cascade(&crc, &prot, sizeof(prot));
    }

    return crc;
}

void Candidate::mutate(
    long rank,
    MutationCounters & mt_counters,
    const CandidateList & candidates) {

    if (rank == -1) {
        // rank -1 is code for destructive randomize
        regrow();
    }
    else if (rank <= CUTOFFS.survivors) {
        this->copy_from(candidates.at(rank));
    }
    else {
        // Replicate mother
        const size_t mother_id = get_dice(CUTOFFS.survivors);
        const Candidate * mother = candidates.at(mother_id);
        this->copy_from(mother);

        const size_t mutation_dice =
            CUTOFFS.survivors +
            get_dice(CUTOFFS.non_survivors);
        if (mutation_dice <= CUTOFFS.cross) {
            // Pick father
            const size_t father_id = get_dice(CUTOFFS.pop_size);
            const Candidate * father = candidates.at(father_id);

            // Fall back to auto mutate if cross fails
            if (!cross_mutate(mother, father)) {
                // Pick a random parent to inherit from and then mutate
                auto_mutate();
                mt_counters.cross_fail++;
            }

            mt_counters.cross++;
        }
        else if (mutation_dice <= CUTOFFS.point) {
            if (!point_mutate()) {
                regrow();
                mt_counters.point_fail++;
            }
            mt_counters.point++;
        }
        else if (mutation_dice <= CUTOFFS.limb) {
            if (!limb_mutate()) {
                regrow();
                mt_counters.limb_fail++;
            }
            mt_counters.limb++;
        }
        else {
            // Individuals not covered by specified mutation
            // rates undergo random destructive mutation
            regrow();
            mt_counters.rand++;
        }
    }
}

void Candidate::copy_from(const Candidate * other) {
    *this = *other;
}

bool Candidate::PtrComparator(const Candidate * lhs, const Candidate * rhs) {
    return lhs->get_score() < rhs->get_score();
};

}  /* elfin */