#include "debug_utils.h"

#include "jutil.h"
#include "stack_trace.h"

namespace elfin {

void __debug(
    bool const predicate,
    const std::string& cond_expr,
    char const* filename,
    int const line,
    const std::string& msg) {
    if (predicate) {
        #pragma omp single
        {
            fprintf(stderr, "\n"
            "---------------------------------------------\n"
            "--------------------ABORT--------------------\n"
            "---------------------------------------------\n" COLOR_RED);
            JUtil.error("Reason: %s at %s:%d\n", msg.c_str(), filename, line);
            JUtil.error("  \"%s\" evaluated to true\n", cond_expr.c_str());
            print_stacktrace();

            JUtil.error("Failure at %s\n", __PRETTY_FUNCTION__);
            throw ExitException{1};
        }
    }
}

}  /* elfin */