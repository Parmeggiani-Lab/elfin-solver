#ifndef TERMINUS_TYPE_H_
#define TERMINUS_TYPE_H_

namespace elfin {

#define FOREACH_TERMINUSTYPE(MACRO) \
    MACRO(N) \
    MACRO(C) \
    MACRO(ANY) \
    MACRO(ENUM_COUNT) \

GEN_ENUM_AND_STRING(TerminusType, TerminusTypeNames, FOREACH_TERMINUSTYPE);

static_assert(TerminusType::ENUM_COUNT == 3);

}  /* elfin */

#endif  /* end of include guard: TERMINUS_TYPE_H_ */