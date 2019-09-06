#include "proto_chain.h"

#include "debug_utils.h"
#include "exceptions.h"
#include "proto_module.h"

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
        throw BadTerminus(TermTypeToCStr(term));
    }
}

void ProtoChain::configure(std::string const& mod_name) {
    n_term_.configure(mod_name, name, TermType::N);
    c_term_.configure(mod_name, name, TermType::C);
}

}  /* elfin */