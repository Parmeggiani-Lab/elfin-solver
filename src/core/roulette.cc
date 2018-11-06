#include "roulette.h"

#include "pair_relationship.h"
#include "jutil.h"

namespace elfin {

IdPairs link_counts_;
std::vector<size_t> cterm_roulette_;
std::vector<size_t> nterm_roulette_;
const IdPairs & LINK_COUNTS = link_counts_;
const std::vector<size_t> & CTERM_ROULETTE = cterm_roulette_;
const std::vector<size_t> & NTERM_ROULETTE = nterm_roulette_;

void init_counts() {
    // Compute neighbour counts
    const size_t dim = REL_MAT.size();
    link_counts_.resize(dim, IdPair());
    for (size_t i = 0; i < dim; i++)
    {
        size_t lhs = 0, rhs = 0;
        for (size_t j = 0; j < dim; j++)
        {
            if (REL_MAT.at(j).at(i) != NULL)
                lhs++;
            if (REL_MAT.at(i).at(j) != NULL)
                rhs++;
        }

        link_counts_.at(i) = IdPair(lhs, rhs);
    }
}

void init_roulettes() {
    init_counts();

    cterm_roulette_.clear();
    nterm_roulette_.clear();
    for (size_t i = 0; i < REL_MAT.size(); i++) {
        for (size_t j = 0; j < link_counts_.at(i).y; j++)
            cterm_roulette_.push_back(i);

        for (size_t j = 0; j < link_counts_.at(i).x; j++)
            nterm_roulette_.push_back(i);
    }
}


}  /* elfin */