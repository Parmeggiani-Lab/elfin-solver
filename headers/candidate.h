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

#define COLLISION_MEASURE max_heavy

namespace elfin {

class Candidate;
typedef std::vector<Candidate *> CandidateList;

class Candidate {
public:
    struct Node;
    typedef std::vector<Node> NodeList;
    typedef NodeList::const_iterator NodeListCItr;

    typedef struct Node {
        const Module * prototype;
        Vector3f com;

        Node(const Module * _prototype, float _x, float _y, float _z) :
            prototype(_prototype), com(_x, _y, _z) {}
        Node() {};
        std::string to_string() const;
        std::string to_csv_string() const;

        static bool collides(
            const Vector3f & new_com,
            const NodeListCItr nodes_begin,
            const NodeListCItr nodes_end) {
            for (NodeListCItr it = nodes_begin; it != nodes_end; ++it) {
                const float com_dist = it->com.sq_dist_to(new_com);
                const float required_com_dist = it->prototype->radii().COLLISION_MEASURE +
                                                it->prototype->radii().COLLISION_MEASURE;
                if (com_dist < (required_com_dist * required_com_dist))
                    return true;
            }

            return false;
        }
    } Node;

    typedef struct {
        size_t max = 0;
    } Lengths;

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
    static auto PtrComparator = [](const Candidate * lhs, const Candidate * rhs) {
        return lhs->get_score() < rhs->get_score();
    };
};

extern const Candidate::Lengths & CANDIDATE_LENGTHS; // defined in population.cc


} /* namespace elfin */

#endif  /* end of include guard: CANDIDATE_H_ */