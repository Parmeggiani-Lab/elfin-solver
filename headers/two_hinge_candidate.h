#ifndef TWO_HINGE_CANDIDATE_H_
#define TWO_HINGE_CANDIDATE_H_

#include "candidate.h"

namespace elfin {

class TwoHingeCandidate : public Candidate {
protected:
    virtual bool cross_mutate(const Candidate * father) {}
    virtual bool point_mutate() {}
    virtual bool limb_mutate() {}
    virtual void grow() = 0;
public:
    virtual void score(const WorkArea & wa) {}
    virtual TwoHingeCandidate * clone() const { return nullptr; }
    virtual void copy_from() const { die("unimplemented"); }
};

}  /* elfin */

#endif  /* end of include guard: TWO_HINGE_CANDIDATE_H_ */