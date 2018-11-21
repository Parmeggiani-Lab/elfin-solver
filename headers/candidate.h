#ifndef CANDIDATE_H_
#define CANDIDATE_H_

#include <vector>
#include <string>
#include <deque>

#include "geometry.h"
#include "checksum.h"
#include "work_area.h"
#include "counter_structs.h"
#include "string_types.h"
#include "module.h"
#include "node_list.h"

namespace elfin {

class Candidate;
typedef std::vector<Candidate *> CandidateList;

class Candidate {
protected:
    /* data members */
    NodeList nodes_;
    float score_ = NAN;

public:
    /* getters */
    float get_score() const { return score_; }
    const NodeList & nodes() const { return nodes_; }
    StrList get_node_names() const;

    /* strings */
    virtual std::string to_string() const;
    virtual std::string to_csv_string() const;

    virtual Crc32 checksum() const;
    virtual void score(const WorkArea & wa) = 0;
    virtual void mutate(
        long rank,
        MutationCounters & mt_counters,
        const CandidateList & candidates) = 0;

    /* ctors & dtors */
    Candidate() {}
    virtual Candidate * clone() const = 0;
    virtual ~Candidate() {}

    /* other methods */
    bool operator<(const Candidate & rhs) const { return score_ < rhs.score_; }
    static bool PtrComparator(const Candidate *, const Candidate *);
};


} /* namespace elfin */

#endif  /* end of include guard: CANDIDATE_H_ */