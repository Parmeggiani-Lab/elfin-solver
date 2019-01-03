#include "debug_utils.h"

#include "jutil.h"
#include "stack_trace.h"

namespace elfin {

void _debug(
    std::string const& cond_expr,
    char const* filename,
    int const line,
    char const* function,
    std::string const& msg) {
    #pragma omp single
    {
        fprintf(stderr, "\n" COLOR_RED
        "---------------------------------------------\n"
        "--------------------ABORT--------------------\n"
        "---------------------------------------------\n" COLOR_RED);
        JUtil.error("Condition: \"%s\" was true\n", cond_expr.c_str());
        JUtil.error("Reason:    %s\n", msg.c_str());
        JUtil.error("Location:  %s:%d in %s\n", filename, line, function);
        print_stacktrace();
        throw ExitException(1, msg.c_str());
    }
}

}  /* elfin */