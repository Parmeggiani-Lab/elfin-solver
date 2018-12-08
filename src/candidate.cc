#include "candidate.h"

#include <sstream>

#include "random_utils.h"
#include "input_manager.h"

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

    for (const auto node_ptr : node_team_->nodes()) {
        const float sq_com_dist = node_ptr->tx().collapsed().sq_dist_to(new_com);
        const float required_com_dist = mod_radius +
                                        node_ptr->prototype()->radius;
        if (sq_com_dist < (required_com_dist * required_com_dist)) {
            return true;
        }
    }

    return false;
}

void Candidate::release_resources() {
    delete node_team_;
}

/* public */

Candidate::Candidate(NodeTeam * node_team) :
    node_team_(node_team) {
    DEBUG(nullptr == node_team);
}

Candidate::Candidate(const Candidate & other) {
    *this = other;
}

Candidate::Candidate(Candidate && other) :
    node_team_(other.node_team_),
    score_(other.score_) {
    other.node_team_ = nullptr;
}

Candidate & Candidate::operator=(const Candidate & other) {
    release_resources();
    DEBUG(nullptr == other.node_team_);
    node_team_ = other.node_team_->clone();
    score_ = other.score_;
    return *this;
}

Candidate::~Candidate() {
    release_resources();
}

bool Candidate::PtrComparator(
    const Candidate * lhs,
    const Candidate * rhs) {
    return lhs->get_score() < rhs->get_score();
}

// static
void Candidate::setup(const WorkArea & wa) {
    /*
     * Calculate expected length as sum of point
     * displacements over avg pair module distance
     */
    float sum_dist = 0.0f;
    const V3fList shape = wa.to_points();
    for (auto i = shape.begin() + 1; i != shape.end(); ++i) {
        sum_dist += (i - 1)->dist_to(*i);
    }

    // Add one because division counts segments. We want number of points.
    const size_t expected_len = 1 + round(sum_dist / OPTIONS.avg_pair_dist);
    MAX_LEN_ = expected_len + OPTIONS.len_dev_alw;
}

void Candidate::mutate(
    size_t rank,
    MutationCounters & mt_counters,
    const CandidateList * candidates) {

    if (rank <= CUTOFFS.survivors) {
        *this = *(candidates->at(rank));
    }
    else {
        // Replicate mother
        const size_t mother_id = get_dice(CUTOFFS.survivors);
        const Candidate * mother = candidates->at(mother_id);
        *this = *(mother);

        const size_t mutation_dice =
            CUTOFFS.survivors +
            get_dice(CUTOFFS.non_survivors);
        if (mutation_dice <= CUTOFFS.cross) {
            // Pick father
            const size_t father_id = get_dice(CUTOFFS.pop_size);
            const NodeTeam * father_team = candidates->at(father_id)->node_team();
            const NodeTeam * mother_team = mother->node_team();

            // Fall back to auto mutate if cross fails
            if (!node_team_->cross_mutate(mother_team, father_team)) {
                // Pick a random parent to inherit from and then mutate
                node_team_->auto_mutate();
                mt_counters.cross_fail++;
            }

            mt_counters.cross++;
        }
        else if (mutation_dice <= CUTOFFS.point) {
            if (!node_team_->point_mutate()) {
                node_team_->regrow();
                mt_counters.point_fail++;
            }
            mt_counters.point++;
        }
        else if (mutation_dice <= CUTOFFS.limb) {
            if (!node_team_->limb_mutate()) {
                node_team_->regrow();
                mt_counters.limb_fail++;
            }
            mt_counters.limb++;
        }
        else {
            // Individuals not covered by specified mutation
            // rates undergo random destructive mutation
            node_team_->regrow();
            mt_counters.rand++;
        }
    }
}

std::string Candidate::to_string() const {
    return string_format("Candidate[%s]", node_team_->to_string().c_str());
}

std::string Candidate::to_csv_string() const {
    std::stringstream ss;

    for (auto n : node_team_->nodes()) {
        ss << n->to_csv_string() << std::endl;
    }

    return ss.str();
}

}  /* elfin */