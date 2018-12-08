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
enum PointMutateMode {
    SwapMode,
    InsertMode,
    DeleteMode,
    EnumSize
};

class Candidate {
protected:
    /* data */
    static size_t MAX_LEN_;
    NodeTeam * node_team_ = nullptr;
    float score_ = NAN;

    /* ctors */
    Candidate() {}

    /* accessors */
    bool collides(
        const Vector3f & new_com,
        const float mod_radius) const;

    /* modifiers */
    void release_resources();

public:
    /* data */
    static const size_t & MAX_LEN; // refers to MAX_LEN_ (private static)

    /* ctors */
    Candidate(NodeTeam * node_team);
    Candidate(const Candidate & other);
    Candidate(Candidate && other);
    Candidate & operator=(const Candidate & other);
    virtual Candidate * clone() const = 0;

    /* dtors */
    virtual ~Candidate();

    /* accessors */
    const NodeTeam * node_team() const { return node_team_; }
    virtual Crc32 checksum() const = 0;
    size_t size() const { return node_team_->size(); }
    float get_score() const { return score_; }
    bool operator<(const Candidate & rhs) const {
        return score_ < rhs.score_;
    }
    static bool PtrComparator(
        const Candidate *,
        const Candidate *);

    /* modifiers */
    static void setup(const WorkArea & wa);
    virtual void score(const WorkArea * wa) = 0;
    void randomize() { node_team_->regrow(); }
    void mutate(
        size_t rank,
        MutationCounters & mt_counters,
        const CandidateList * candidates);

    /* printers */
    virtual std::string to_string() const;
    virtual std::string to_csv_string() const;
    virtual StrList get_node_names() const {
        return node_team_->get_node_names();
    }
};


} /* namespace elfin */

#endif  /* end of include guard: CANDIDATE_H_ */