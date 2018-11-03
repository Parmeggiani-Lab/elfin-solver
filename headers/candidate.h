#ifndef CANDIDATE_H_
#define CANDIDATE_H_

#include <vector>
#include <string>

#include "geometry.h"
#include "checksum.h"
#include "work_area.h"
#include "counter_structs.h"

namespace elfin {

typedef struct {
    ulong expected = 0;
    ulong min = 0;
    ulong max = 0;
} CandidateLengths;

class Candidate {
public:
    struct Node {
        size_t id;
        Point3f com;

        Node(size_t id, float x, float y, float z) :
            id(id), com(x, y, z) {};
        std::string to_string() const;
        std::string to_csv_string() const;
    };

protected:
    /* data members */
    std::vector<Node> nodes_;
    float score_ = NAN;

public:

    /* getters */
    float get_score() const { return score_; }
    const std::vector<Node> & nodes() const { return nodes_; }
    const std::vector<std::string> get_node_names() const;

    /* strings */
    virtual std::string to_string() const;
    virtual std::string to_csv_string() const;

    virtual Crc32 checksum() const;
    virtual void score(const WorkArea & wa) = 0;
    virtual void mutate(long rank,
        const CandidateLengths & cd_lens,
        const MutationCounters & mt_counters=MutationCounters()) = 0;
    virtual Candidate * new_copy() const = 0;

    /* operators */
    bool operator>(const Candidate & rhs) const { return score_ > rhs.get_score(); }
    bool operator<(const Candidate & rhs) const { return score_ < rhs.get_score(); }
};

typedef std::vector<Candidate::Node>::const_iterator ConstNodeIterator;

} /* namespace elfin */

#endif  /* end of include guard: CANDIDATE_H_ */