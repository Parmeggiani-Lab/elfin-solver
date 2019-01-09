#include "proto_chain.h"

#include "debug_utils.h"
#include "exceptions.h"

namespace elfin {

/* public */
ProtoTerm const& ProtoChain::get_term(TermType const term) const {
    if (term == TermType::N) {
        return n_term_;
    }
    else if (term == TermType::C) {
        return c_term_;
    }
    else {
        bad_term(term);
        throw ShouldNotReach();
    }
}

void ProtoChain::configure() {
    n_term_.configure();
    c_term_.configure();
}

}  /* elfin */