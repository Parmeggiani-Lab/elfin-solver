#include "term_type.h"

#include "exceptions.h"

namespace elfin {

std::unordered_set<std::string> const VALID_TERM_NAMES = {
    "n",
    "c",
    "N",
    "C"
};

TermType opposite_term(TermType const term) {
    if (term == TermType::N) {
        return TermType::C;
    }
    else if (term == TermType::C) {
        return TermType::N;
    }
    else {
        throw BadTerminus(TermTypeToCStr(term));
    }
}

TermType parse_term(std::string const& str) {
    if (str == "n" or str == "N") {
        return TermType::N;
    }
    else if (str == "c" or str == "C") {
        return TermType::C;
    }
    else {
        throw BadArgument(
            "Cannot parse terminut type from: \"" + str + "\".");
    }
}

}  /* elfin */