#ifndef SPEC_PARSER_H_
#define SPEC_PARSER_H_

#include "json.h"
#include "spec.h"

namespace elfin {

class SpecParser
{
public:
    SpecParser() {};
    Spec parse(const std::string & filepath);
};

}

#endif /* end of include guard: SPEC_PARSER_H_ */