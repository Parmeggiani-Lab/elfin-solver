#include "proto_chain.h"

#include "debug_utils.h"
#include "exit_exception.h"

// #define PRINT_FINALIZE

namespace elfin {

/* public */
ProtoTerm const& ProtoChain::get_term(
    TermType const term) const {
    if (term == TermType::N) {
        return n_term_;
    }
    else if (term == TermType::C) {
        return c_term_;
    }
    else {
        bad_term(term);
        throw ExitException{1};  // Suppress warning.
    }
}

void ProtoChain::finalize() {
    TRACE_NOMSG(already_finalized_);
    already_finalized_ = true;

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
    TermType const term) const {
    return get_term(term).pick_random_link(term);
}

}  /* elfin */