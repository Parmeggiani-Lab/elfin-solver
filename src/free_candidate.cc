#include "free_candidate.h"

#include <sstream>
#include <exception>

#if defined(NO_POINT_MUTATE) || \
    defined(NO_LIMB_MUTATE) || \
    defined(NO_CROSS_MUTATE)
#warning "Mutation code not being compiled!!"
#endif

namespace elfin {
    
/* public */
std::string FreeCandidate::to_string() const {
    std::stringstream ss;

    ss << "FreeCandidate " << this << ":\n";
    ss << Candidate::to_string();

    return ss.str();
}

FreeCandidate * FreeCandidate::clone() const {
    return new FreeCandidate(*this);
}

}  /* elfin */