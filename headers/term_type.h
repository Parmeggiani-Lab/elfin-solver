#ifndef TERM_TYPE_H_
#define TERM_TYPE_H_

#include <string>
#include <unordered_set>

#include "jutil.h"
#include "debug_utils.h"

namespace elfin {

#define FOREACH_TERMTYPE(MACRO) \
    MACRO(N) \
    MACRO(C) \
    MACRO(ANY) \
    MACRO(NONE) \
    MACRO(_ENUM_COUNT)
GEN_ENUM_AND_STRING(TermType, TermTypeNames, FOREACH_TERMTYPE);

extern std::unordered_set<std::string> const VALID_TERM_NAMES;

TermType opposite_term(TermType const term);

TermType parse_term(std::string const& str);

}  /* elfin */

#endif  /* end of include guard: TERM_TYPE_H_ */