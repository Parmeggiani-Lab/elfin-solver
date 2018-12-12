#ifndef DEBUG_UTILS_H_
#define DEBUG_UTILS_H_

#include <string>

#include "jutil.h"
#include "string_utils.h"

namespace elfin {

#define DEBUG1(COND_EXPR) do { __debug(COND_EXPR, #COND_EXPR, __FILE__, __LINE__, "No Message"); } while(0)
#define DEBUG2(COND_EXPR, MSG) do { __debug(COND_EXPR, #COND_EXPR, __FILE__, __LINE__, MSG); } while(0)
#define GET_DEBUG_MACRO(_1,_2,NAME,...) NAME

#ifdef NDEBUG
#define DEBUG(...)
#else
#define DEBUG(...) GET_DEBUG_MACRO(__VA_ARGS__, DEBUG2, DEBUG1)(__VA_ARGS__)
#endif  /* ifndef NDEBUG */

#define NICE_PANIC(...) GET_DEBUG_MACRO(__VA_ARGS__, DEBUG2, DEBUG1)(__VA_ARGS__)
#define UNIMPLEMENTED() NICE_PANIC("Unimplemented", __PRETTY_FUNCTION__)
#define PROB_FUNC() do { wrn("%s called at %s:%d\n", __PRETTY_FUNCTION__, __FILE__, __LINE__); } while(0)

void __debug(
    bool const result,
    const std::string & cond_expr,
    char const* filename,
    int const line,
    const std::string & msg);

}  /* elfin */

#endif  /* end of include guard: DEBUG_UTILS_H_ */