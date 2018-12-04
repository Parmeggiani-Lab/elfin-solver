#include "chain.h"

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
    n_term_.finalize();
    c_term_.finalize();
}

const Link & Chain::pick_random_link(
    const TerminusType term) const {
    return get_term(term).pick_random_link(term);
}

}  /* elfin */