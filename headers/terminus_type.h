#ifndef TERMINUS_TYPE_H_
#define TERMINUS_TYPE_H_

#include <string>

#include "jutil.h"

namespace elfin {

#define FOREACH_TERMINUSTYPE(MACRO) \
    MACRO(N) \
    MACRO(C) \
    MACRO(ANY) \
    MACRO(NONE) \
    MACRO(ENUM_COUNT) \

GEN_ENUM_AND_STRING(TerminusType, TerminusTypeNames, FOREACH_TERMINUSTYPE);

void death_by_bad_terminus(std::string func_name, TerminusType term);

TerminusType random_term();

const TerminusType OPPOSITE_TERM[2] = { TerminusType::C, TerminusType::N };

}  /* elfin */

#endif  /* end of include guard: TERMINUS_TYPE_H_ */