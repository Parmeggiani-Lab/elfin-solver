#include "chain.h"

#include "debug_utils.h"

// #define PRINT_FINALIZE

namespace elfin {

/* public */
const Terminus & Chain::get_term(
    const TerminusType term) const {
    if (term == TerminusType::N) {
        return n_term_;
    }
    else if (term == TerminusType::C) {
        return c_term_;
    }
    else {
        death_by_bad_terminus(__PRETTY_FUNCTION__, term); // Aborts
        exit(1); // To suppress warning
    }
}

void Chain::finalize() {
    NICE_PANIC(finalized_,
             string_format("%s called more than once!", __PRETTY_FUNCTION__).c_str());
    finalized_ = true;

#ifdef PRINT_FINALIZE
    wrn("Finalizing chain %s N term\n", name.c_str());
#endif  /* ifdef PRINT_FINALIZE */

    n_term_.finalize();

#ifdef PRINT_FINALIZE
    wrn("Finalizing chain %s C term\n", name.c_str());
#endif  /* ifdef PRINT_FINALIZE */

    c_term_.finalize();
}

const Link & Chain::pick_random_link(
    const TerminusType term) const {
    return get_term(term).pick_random_link(term);
}

}  /* elfin */