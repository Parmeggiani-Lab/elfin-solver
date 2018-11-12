#ifndef CANDIDATE_H_
#define CANDIDATE_H_

#include <vector>
#include <string>
#include <deque>

#include "geometry.h"
#include "checksum.h"
#include "work_area.h"
#include "counter_structs.h"

namespace elfin {

class Candidate;
typedef std::vector<Candidate *> Candidates;

class Candidate {
public:
    typedef struct Node {
        size_t id;
        Point3f com;

        Node(size_t id, float x, float y, float z) :
            id(id), com(x, y, z) {};
        Node() {};
        std::string to_string() const;
        std::string to_csv_string() const;
    } Node;

    typedef struct {
        ulong expected = 0;
        ulong min = 0;
        ulong max = 0;
    } Lengths;

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
    virtual void mutate(
        long rank,
        MutationCounters & mt_counters,
        const Candidates & candidates) = 0;

    /* ctors & dtors */
    Candidate() {}
    virtual Candidate * clone() const = 0;
    virtual ~Candidate() {}

    /* operators */
    bool operator<(const Candidate & rhs) const { return score_ < rhs.score_; }
};

extern const Candidate::Lengths & CANDIDATE_LENGTHS; // defined in population.cc
typedef std::vector<Candidate::Node> Nodes;
typedef std::vector<Candidate::Node>::const_iterator ConstNodeIterator;
auto CandidatePtrComparator = [ ](const Candidate * lhs, const Candidate * rhs) {
    return lhs->get_score() < rhs->get_score();
};


} /* namespace elfin */

#endif  /* end of include guard: CANDIDATE_H_ */