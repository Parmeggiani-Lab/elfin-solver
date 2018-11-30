#ifndef ONE_HINGE_CANDIDATE_H_
#define ONE_HINGE_CANDIDATE_H_

#include "candidate.h"

namespace elfin {

class OneHingeCandidate : public Candidate {
protected:
    virtual bool cross_mutate(const Candidate * father) {}
    virtual bool point_mutate() {}
    virtual bool limb_mutate() {}
    virtual void grow() = 0;
public:
    virtual void score(const WorkArea & wa) {}
    virtual OneHingeCandidate * clone() const { return nullptr; }
    virtual void copy_from() const { die("unimplemented"); }
};

}  /* elfin */

#endif  /* end of include guard: ONE_HINGE_CANDIDATE_H_ */