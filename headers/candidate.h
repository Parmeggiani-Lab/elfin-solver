#ifndef CANDIDATE_H_
#define CANDIDATE_H_

#include <vector>
#include <string>
#include <deque>

#include "geometry.h"
#include "checksum.h"
#include "work_area.h"
#include "mutation_modes.h"
#include "node_team.h"

namespace elfin {

class Candidate;
typedef std::vector<Candidate *> CandidateList;

class Candidate {
protected:
    /* data */
    static size_t MAX_LEN_;
    NodeTeam* node_team_ = nullptr;
    float score_ = NAN;

    /* modifiers */
    void release_resources();

public:
    /* data */
    static size_t const& MAX_LEN; // refers to MAX_LEN_ (private static)

    /* ctors */
    Candidate() {} // needed in order to support vector.resize()
    Candidate(WorkType const work_type);
    Candidate(Candidate const& other);
    Candidate(Candidate && other);
    Candidate* clone() const;

    /* dtors */
    virtual ~Candidate();

    /* accessors */
    NodeTeam const* node_team() const { return node_team_; }
    virtual Crc32 checksum() const { return node_team_->checksum(); }
    size_t size() const { return node_team_->size(); }
    float get_score() const { return score_; }
    bool operator<(Candidate const& rhs) const {
        return score_ < rhs.score_;
    }
    static bool PtrComparator(
        const Candidate *,
        const Candidate *);

    /* modifiers */
    Candidate& operator=(Candidate const& other);
    static void setup(WorkArea const& wa);
    void calc_score(WorkArea const* wa) { score_ = node_team_->score(wa); }
    void randomize() { node_team_->randomize(); }
    MutationMode mutate(
        size_t const rank,
        CandidateList const* candidates);

    /* printers */
    virtual std::string to_string() const;
    virtual std::string to_csv_string() const;
    StrList get_node_names() const {
        return node_team_->get_node_names();
    }
};


} /* namespace elfin */

#endif  /* end of include guard: CANDIDATE_H_ */