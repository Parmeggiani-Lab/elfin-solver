#include "debug_utils.h"

#include "stack_trace.h"

namespace elfin {

void __debug(
    bool const result,
    const std::string& cond_expr,
    char const* filename,
    int const line,
    const std::string& msg) {
    if (result) {
        raw("\n\n");
        err("Bug: %s\n", msg.c_str());
        err("Where: %s:%d\n", filename, line);
        err("Reason: \"%s\" evaluated to true\n", cond_expr.c_str());
        print_stacktrace();
        die("Exit by call to %s\n", __PRETTY_FUNCTION__);
    }
}

}  /* elfin */