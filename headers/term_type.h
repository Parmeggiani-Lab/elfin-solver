#ifndef TERM_TYPE_H_
#define TERM_TYPE_H_

#include <string>

#include "random_utils.h"
#include "debug_utils.h"

namespace elfin {

#define FOREACH_TERMTYPE(MACRO) \
    MACRO(N) \
    MACRO(C) \
    MACRO(ANY) \
    MACRO(NONE) \
    MACRO(_ENUM_COUNT) \

GEN_ENUM_AND_STRING(TermType, TermTypeNames, FOREACH_TERMTYPE);

inline void bad_term(TermType term) {
    TRACE(term == term,
          "Bad TermType: %s\n",
          TermTypeToCStr(term));
}

inline TermType opposite_term(TermType const term) {
    if (term == TermType::N) {
        return TermType::C;
    }
    else if (term == TermType::C) {
        return TermType::N;
    }
    else {
        bad_term(term);
        return TermType::NONE;
    }
}

}  /* elfin */

#endif  /* end of include guard: TERM_TYPE_H_ */