#include "proto_chain.h"

#include "debug_utils.h"

// #define PRINT_FINALIZE

namespace elfin {

/* public */
const ProtoTerminus & ProtoChain::get_term(
    const TerminusType term) const {
    if (term == TerminusType::N) {
        return n_term_;
    }
    else if (term == TerminusType::C) {
        return c_term_;
    }
    else {
        bad_terminus(term);
    }
}

void ProtoChain::finalize() {
    NICE_PANIC(finalized_,
               string_format("%s called more than once!", __PRETTY_FUNCTION__).c_str());
    finalized_ = true;

#ifdef PRINT_FINALIZE
    wrn("Finalizing proto_chain %s N term\n", name.c_str());
#endif  /* ifdef PRINT_FINALIZE */

    n_term_.finalize();

#ifdef PRINT_FINALIZE
    wrn("Finalizing proto_chain %s C term\n", name.c_str());
#endif  /* ifdef PRINT_FINALIZE */

    c_term_.finalize();
}

const ProtoLink & ProtoChain::pick_random_proto_link(
    const TerminusType term) const {
    return get_term(term).pick_random_proto_link(term);
}

}  /* elfin */