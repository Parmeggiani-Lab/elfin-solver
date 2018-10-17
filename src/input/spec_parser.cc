#include "spec_parser.h"

namespace elfin {

Spec SpecParser::parse(const std::string & filepath) {
    const JSON j = parse_json(filepath);
    
    return Spec(j);
}

} /* elfin */