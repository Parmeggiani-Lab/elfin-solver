#include "candidate.h"

#include <sstream>

#include "random_utils.h"
#include "input_manager.h"
#include "basic_node_team.h"

namespace elfin {

/* static data members */
size_t Candidate::MAX_LEN_ = 0;
size_t const& Candidate::MAX_LEN = Candidate::MAX_LEN_;

/* protected */

/* accessors */

/* modifiers */
void Candidate::release_resources() {
    delete node_team_;
}

/* public */

/* ctors */
Candidate::Candidate(const WorkType work_type) {
    switch (work_type) {
    case WorkType::FREE:
        node_team_ = new BasicNodeTeam();
        break;
    // case WorkType::ONE_HINGE:
    //     node_team_ = new OneHingeNodeTeam();
    //     break;
    // case WorkType::TWO_HINGE:
    //     node_team_ = new TwoHingeNodeTeam();
    //     break;
    default:
        bad_work_type(work_type);
    }

    DEBUG(node_team_ == nullptr);
}

Candidate::Candidate(Candidate const& other) {
    *this = other; // calls operator=(T const&)
}

Candidate::Candidate(Candidate && other) {
     *this = other; // calls operator=(T &&)
}

Candidate* Candidate::clone() const {
    Candidate* new_cand = new Candidate();
    new_cand->node_team_ = node_team_->clone();
    return new_cand;
}

/* dtors */
Candidate::~Candidate() {
    release_resources();
}

/* accessors */
bool Candidate::PtrComparator(
    Candidate const* lhs,
    Candidate const* rhs) {
    return lhs->score() < rhs->score();
}

/* modifiers */
Candidate& Candidate::operator=(Candidate const& other) {
    if (this != &other) {
        release_resources();
        DEBUG(nullptr == other.node_team_);
        node_team_ = other.node_team_->clone();
    }
    return *this;
}

Candidate& Candidate::operator=(Candidate && other) {
    if (this != &other) {
        release_resources();
        DEBUG(nullptr == other.node_team_);
        node_team_ = other.node_team_;
        other.node_team_ = nullptr;
    }
    return *this;
}

// static
void Candidate::setup(WorkArea const& wa) {
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
    const size_t expected_len =
        1 + round(sum_dist / OPTIONS.avg_pair_dist);
    MAX_LEN_ = expected_len + OPTIONS.len_dev_alw;
}

MutationMode Candidate::mutate_and_score(
    const size_t rank,
    CandidateList const* candidates,
    WorkArea const* wa) {

    MutationMode mode = MutationMode::NONE;
    if (rank < CUTOFFS.survivors) { // use < because rank is 0-indexed
        *this = *(candidates->at(rank));
        mode = MutationMode::NONE;
    }
    else {
        // Replicate mother
        const size_t mother_id =
            random::get_dice(CUTOFFS.survivors); // only elites
        NodeTeam const* mother_team = candidates->at(mother_id)->node_team();

        const size_t father_id =
            random::get_dice(CUTOFFS.pop_size); // include all candidates
        NodeTeam const* father_team = candidates->at(father_id)->node_team();

        mode = node_team_->mutate_and_score(
                   mother_team,
                   father_team,
                   wa);
    }

    return mode;
}

/* printers */
std::string Candidate::to_string() const {
    return string_format(
               "Candidate[\n%s\n]",
               node_team_->to_string().c_str());
}

std::string Candidate::to_csv_string() const {
    std::ostringstream ss;

    for (auto n : node_team_->nodes()) {
        ss << n->to_csv_string() << std::endl;
    }

    return ss.str();
}

}  /* elfin */