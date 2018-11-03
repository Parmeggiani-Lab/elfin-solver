#ifndef CANDIDATE_H_
#define CANDIDATE_H_

#include <vector>
#include <string>

#include "shorthands.h"
#include "geometry.h"
#include "checksum.h"
#include "work_area.h"
#include "counter_structs.h"

namespace elfin {

class Candidate {

protected:
    class Node {
    public:
        uint id_;
        Point3f com_;

        std::string to_string(const IdNameMap & inm) const;
        std::string to_csv_string() const;
    };

    /* data members */
    std::vector<Node> nodes_;
    float score_ = NAN;

public:
    /* getters */
    float get_score() const { return score_; }
    const std::vector<Node> & nodes() const { return nodes_; }

    /* strings */
    virtual std::string to_string(const IdNameMap & inm) const;
    virtual std::string to_csv_string() const;

    virtual void init(const WorkArea & wa) = 0;
    virtual void score(const WorkArea & wa) = 0;
    virtual void mutate(
        size_t rank, 
        const MutationCutoffs & mt_cutoffs,
        const MutationCounters & mt_counts) = 0;
    virtual Crc32 checksum() const;

    /* operators */
    bool operator>(const Candidate & rhs) const { return score_ > rhs.get_score(); }
    bool operator<(const Candidate & rhs) const { return score_ < rhs.get_score(); }
};

} /* namespace elfin */

#endif  /* end of include guard: CANDIDATE_H_ */