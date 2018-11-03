#ifndef ROULETTE_H_
#define ROULETTE_H_

#include "id_pair.h"

namespace elfin {

extern const IdPairs LINK_COUNTS;
extern const std::vector<size_t> CTERM_ROULETTE;
extern const std::vector<size_t> NTERM_ROULETTE;

void init_roulettes();

}  /* elfin */

#endif  /* end of include guard: ROULETTE_H_ */