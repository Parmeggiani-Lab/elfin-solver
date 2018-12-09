#ifndef CANDIDATE_H_
#define CANDIDATE_H_

#include <vector>
#include <string>
#include <deque>

#include "geometry.h"
#include "checksum.h"
#include "work_area.h"
#include "mutation_counters.h"
#include "node_team.h"

namespace elfin {

class Candidate;
typedef std::vector<Candidate *> CandidateList;

class Candidate {
protected:
    /* data */
    static size_t MAX_LEN_;
    NodeTeam * node_team_ = nullptr;
    float score_ = NAN;

    /* ctors */

    /* accessors */

    /* modifiers */
    void release_resources();

public:
    /* data */
    static const size_t & MAX_LEN; // refers to MAX_LEN_ (private static)

    /* ctors */
    Candidate() {} // needed in order to support vector.resize()
    Candidate(NodeTeam * node_team);
    Candidate(const Candidate & other);
    Candidate(Candidate && other);
    Candidate * clone() const;

    /* dtors */
    virtual ~Candidate();

    /* accessors */
    const NodeTeam * node_team() const { return node_team_; }
    virtual Crc32 checksum() const { return node_team_->checksum(); }
    size_t size() const { return node_team_->size(); }
    float get_score() const { return score_; }
    bool operator<(const Candidate & rhs) const {
        return score_ < rhs.score_;
    }
    static bool PtrComparator(
        const Candidate *,
        const Candidate *);

    /* modifiers */
    Candidate & operator=(const Candidate & other);
    static void setup(const WorkArea & wa);
    void calc_score(const WorkArea * wa) { score_ = node_team_->score(wa); }
    void randomize() { node_team_->randomize(); }
    void mutate(
        const size_t rank,
        MutationCounters & mt_counters,
        const CandidateList * candidates);

    /* printers */
    virtual std::string to_string() const;
    virtual std::string to_csv_string() const;
    StrList get_node_names() const {
        return node_team_->get_node_names();
    }
};


} /* namespace elfin */

#endif  /* end of include guard: CANDIDATE_H_ */