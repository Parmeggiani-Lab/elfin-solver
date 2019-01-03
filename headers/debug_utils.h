#ifndef DEBUG_UTILS_H_
#define DEBUG_UTILS_H_

#include <string>

#include "string_utils.h"
#include "exceptions.h"

namespace elfin {

#define TRACE(COND_EXPR, ...) \
    do {\
        if (COND_EXPR) {\
            _debug(#COND_EXPR, __FILE__, __LINE__, __PRETTY_FUNCTION__, string_format(__VA_ARGS__));\
        }\
    } while(0)

#define TRACE_NOMSG(COND_EXPR) \
    do {\
        if (COND_EXPR) {\
            _debug(#COND_EXPR, __FILE__, __LINE__, __PRETTY_FUNCTION__, #COND_EXPR);\
        }\
    } while(0)

#ifdef NDEBUG
#define DEBUG(...)
#define DEBUG_NOMSG(...)
#else
#define DEBUG(COND_EXPR, ...) TRACE(COND_EXPR, __VA_ARGS__)
#define DEBUG_NOMSG(COND_EXPR) TRACE_NOMSG(COND_EXPR)
#endif  /* ifndef NDEBUG */

#define UNIMP() DEBUG("Unimplemented", "Unimplemented")
#define PROBE_FUNC() \
    do {\
        JUtil.warn(\
            "%s called at %s:%d\n",\
            __PRETTY_FUNCTION__,\
            __FILE__,\
            __LINE__);\
    } while(0)
#define PANIC(...) \
    do {\
        _Pragma("omp single")\
        {\
            std::string const& reason = string_format(__VA_ARGS__);\
            JUtil.error(reason.c_str());\
            throw ExitException(1, reason);\
        }\
    } while(0)
#define PANIC_IF(PREDICATE, ...) \
    do {\
        if(PREDICATE) {\
            _Pragma("omp single")\
            {\
                std::string const& reason = string_format(__VA_ARGS__);\
                JUtil.error(reason.c_str());\
                throw ExitException(1, reason);\
            }\
        }\
    } while(0)
#define JSON_LOG_EXIT(JSON_EX) \
    do {\
        JUtil.error("JSON parse failure due to: %s\n", JSON_EX.what());\
        JUtil.error("Occured at %s:%zu\n", __FILE__, __LINE__);\
        JUtil.error("Function %s\n", __PRETTY_FUNCTION__);\
        JUtil.error("Check https://nlohmann.github.io/json/classnlohmann_1_1basic__json.html\n");\
        throw JSON_EX;\
    } while(0)

void _debug(
    std::string const& cond_expr,
    char const* filename,
    int const line,
    char const* function,
    std::string const& msg);

}  /* elfin */

#endif  /* end of include guard: DEBUG_UTILS_H_ */