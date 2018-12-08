#include "debug_utils.h"

#include "jutil.h"
#include "stack_trace.h"

namespace elfin {

void __debug(
    const bool result,
    const std::string & cond_expr,
    const std::string & msg) {
	if (result) {
        raw("\n\n");
		err("Bug: %s\n", msg.c_str());
		err("Reason: \"%s\" evaluated to true\n", cond_expr.c_str());
		print_stacktrace();
		die("Exit by call to %s\n", __PRETTY_FUNCTION__);
	}
}

}  /* elfin */