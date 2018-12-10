#ifndef MUTATION_MODES_H_
#define MUTATION_MODES_H_

#include <unordered_map>

#include "vector_utils.h"

namespace elfin {

/* types */
#define FOREACH_MUTATION_MODES(MACRO) \
    MACRO(_ENUM_START) \
    MACRO(ERODE) \
    MACRO(DELETE) \
    MACRO(INSERT) \
    MACRO(SWAP) \
    MACRO(CROSS) \
    MACRO(RANDOMIZE) \
    MACRO(_ENUM_SIZE_PLUS_ONE) \

GEN_ENUM_AND_STRING(MutationMode, MutationModeNames, FOREACH_MUTATION_MODES);

typedef std::unordered_map<MutationMode, size_t> MutationCounter;
typedef Vector<MutationMode> MutationModeList;

/* free functions */
MutationModeList gen_mutation_mode_list();
MutationCounter gen_mutation_counter();

inline void bad_mutation_mode(MutationMode mode) {
    NICE_PANIC(mode == mode, string_format("Bad MutationMode: %s\n",
                                           MutationModeNames[mode]));
}

}  /* elfin */

#endif  /* end of include guard: MUTATION_MODES_H_ */