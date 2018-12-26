#ifndef MUTATION_H_
#define MUTATION_H_

#include <unordered_map>

#include "vector_utils.h"

namespace elfin {

namespace mutation {

/* types */
#define FOREACH_MODES(MACRO) \
    MACRO(NONE) \
    MACRO(ERODE) \
    MACRO(DELETE) \
    MACRO(INSERT) \
    MACRO(SWAP) \
    MACRO(CROSS) \
    MACRO(REGENERATE) \
    MACRO(_ENUM_SIZE) \

GEN_ENUM_AND_STRING(Mode, ModeNames, FOREACH_MODES);

typedef std::unordered_map<Mode, size_t> Counter;
typedef Vector<Mode> ModeList;

/* free functions */
ModeList gen_mode_list();
Counter gen_counter();

static inline void bad_mode(Mode mode) {
    NICE_PANIC(mode == mode, string_format("Bad Mutation Mode: %s\n",
                                           ModeToCStr(mode)));
}

}  /* mutation */

}  /* elfin */

#endif  /* end of include guard: MUTATION_H_ */