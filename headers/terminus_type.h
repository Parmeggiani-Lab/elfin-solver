#ifndef TERMINUS_TYPE_H_
#define TERMINUS_TYPE_H_

#include <string>

#include "random_utils.h"
#include "debug_utils.h"

namespace elfin {

#define FOREACH_TERMINUSTYPE(MACRO) \
    MACRO(N) \
    MACRO(C) \
    MACRO(ANY) \
    MACRO(NONE) \
    MACRO(_ENUM_COUNT) \

GEN_ENUM_AND_STRING(TerminusType, TerminusTypeNames, FOREACH_TERMINUSTYPE);

/*
 * Returns either N or C randomly.
 */
inline TerminusType random_termius() {
    return random::get_dice(2) == 0 ? TerminusType::N : TerminusType::C;
}

inline void bad_terminus(TerminusType term) {
    NICE_PANIC(term == term, string_format("Bad TerminusType: %s\n",
                                           TerminusTypeToCStr(term)));
}

inline TerminusType opposite_term(TerminusType const term) {
    if (term == TerminusType::N) {
        return TerminusType::C;
    }
    else if (term == TerminusType::C) {
        return TerminusType::N;
    }
    else {
        bad_terminus(term);
    }
}

}  /* elfin */

#endif  /* end of include guard: TERMINUS_TYPE_H_ */