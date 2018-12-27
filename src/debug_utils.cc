#include "debug_utils.h"

#include <jutil/jutil.h>

#include "stack_trace.h"

namespace elfin {

void __debug(
    bool const result,
    const std::string& cond_expr,
    char const* filename,
    int const line,
    const std::string& msg) {
    if (result) {
        JUtil.log("", "\n\n");
        JUtil.error("Bug: %s\n", msg.c_str());
        JUtil.error("Where: %s:%d\n", filename, line);
        JUtil.error("Reason: \"%s\" evaluated to true\n", cond_expr.c_str());
        print_stacktrace();
        JUtil.panic("Exit by call to %s\n", __PRETTY_FUNCTION__);
    }
}

}  /* elfin */