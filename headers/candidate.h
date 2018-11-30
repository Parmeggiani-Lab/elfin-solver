#ifndef CANDIDATE_H_
#define CANDIDATE_H_

#include <vector>
#include <string>
#include <deque>

#include "input_manager.h"
#include "geometry.h"
#include "checksum.h"
#include "work_area.h"
#include "string_utils.h"
#include "mutation_counters.h"
#include "node.h"
#include "terminus_type.h"

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
    /* types */
    enum Direction { LEFT, RIGHT };

    /* data members */
    static size_t MAX_LEN_;
    NodeDeque nodes_;
    float score_ = NAN;

    /* other methods */
    bool collides(
        const Vector3f & new_com,
        const float mod_radius) const;
    void auto_mutate();

    /*
     * Pick a valid cross-mutate point in both mother and father, then join each
     * side to form the child.
     */
    virtual bool cross_mutate(
        const Candidate * mother,
        const Candidate * father) = 0;

    /*
     * Point Mutation tries the following modifications:
     *   1. Swap with another node
     *   2. Insert a node
     *   3. Delete the node
     *
     * The selection is uniform probability without replacement.
     */
    virtual bool point_mutate() = 0;

    /*
     * Cut off one side of the strand and grow a new "limb".
     * (virtual)
     */
    virtual bool limb_mutate() = 0;

    /*
     * Grows a selected tip until MAX_LEN is reached.
     */
    virtual void grow(const size_t tip_index, TerminusType term) = 0;

    /*
     * Removes all nodes and grow from nothing to MAX_LEN.
     */
    virtual void regrow() = 0;

public:
    /* data memebers */
    static const size_t & MAX_LEN; // refers to MAX_LEN_ (private static)
    static void set_max_len(const size_t l) { MAX_LEN_ = l; }

    /* getters */
    float get_score() const { return score_; }
    const std::deque<Node *> & nodes() const { return nodes_; }
    StrList get_node_names() const;

    /* strings */
    virtual std::string to_string() const;
    virtual std::string to_csv_string() const;

    virtual Crc32 checksum() const;
    virtual void score(const WorkArea & wa) = 0;
    void mutate(
        long rank,
        MutationCounters & mt_counters,
        const CandidateList & candidates);

    /* ctors & dtors */
    Candidate() {}
    Candidate(const Candidate & other);
    Candidate & operator=(const Candidate & other);
    virtual Candidate * clone() const = 0;
    virtual void copy_from(const Candidate * other);
    virtual ~Candidate();

    /* other methods */
    bool operator<(const Candidate & rhs) const { return score_ < rhs.score_; }
    static bool PtrComparator(const Candidate *, const Candidate *);
};


} /* namespace elfin */

#endif  /* end of include guard: CANDIDATE_H_ */