#include "proto_chain.h"

#include "debug_utils.h"
#include "exit_exception.h"

// #define PRINT_FINALIZE

namespace elfin {

/* public */
ProtoTerminus const& ProtoChain::get_term(
    TerminusType const term) const {
    if (term == TerminusType::N) {
        return n_term_;
    }
    else if (term == TerminusType::C) {
        return c_term_;
    }
    else {
        bad_terminus(term);
        throw ExitException{1}; // Suppress warning.
    }
}

void ProtoChain::finalize() {
    TRACE_PANIC(finalized_,
               string_format("%s called more than once!", __PRETTY_FUNCTION__).c_str());
    finalized_ = true;

#ifdef PRINT_FINALIZE
    JUtil.warn("Finalizing proto_chain %s N term\n", name.c_str());
#endif  /* ifdef PRINT_FINALIZE */

    n_term_.finalize();

#ifdef PRINT_FINALIZE
    JUtil.warn("Finalizing proto_chain %s C term\n", name.c_str());
#endif  /* ifdef PRINT_FINALIZE */

    c_term_.finalize();
}

ProtoLink const& ProtoChain::pick_random_link(
    TerminusType const term) const {
    return get_term(term).pick_random_link(term);
}

}  /* elfin */