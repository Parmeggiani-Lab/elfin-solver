#include "free_candidate.h"

#include <sstream>

namespace elfin {

/* strings */
std::string FreeCandidate::to_string(const IdNameMap & inm) const {
    std::stringstream ss;

    ss << "FreeCandidate " << this << ":\n";
    ss << Candidate::to_string(inm);

    return ss.str();
}

void FreeCandidate::init(const WorkArea & wa) {

}

void FreeCandidate::score(const WorkArea & wa) {

}

void FreeCandidate::mutate(size_t rank) {

}


}  /* elfin */