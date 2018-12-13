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
    Crc32 checksum() const { return node_team_->checksum(); }
    size_t size() const { return node_team_->size(); }
    float score() const { return node_team_->score(); }
    bool operator<(Candidate const& rhs) const {
        return score() < rhs.score();
    }
    static bool PtrComparator(
        const Candidate *,
        const Candidate *);

    /* modifiers */
    Candidate& operator=(Candidate const& other);
    Candidate& operator=(Candidate && other);
    static void setup(WorkArea const& wa);
    void randomize() { node_team_->randomize(); }
    MutationMode mutate_and_score(
        size_t const rank,
        CandidateList const* candidates,
        WorkArea const* wa);

    /* printers */
    virtual std::string to_string() const;
    virtual std::string to_csv_string() const;
    StrList get_node_names() const {
        return node_team_->get_node_names();
    }
};


} /* namespace elfin */

#endif  /* end of include guard: CANDIDATE_H_ */