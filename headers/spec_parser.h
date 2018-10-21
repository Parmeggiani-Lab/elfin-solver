#ifndef SPEC_PARSER_H_
#define SPEC_PARSER_H_

#include <memory>

#include "json.h"
#include "spec.h"

namespace elfin {

class SpecParser
{
public:
    static const char * const pg_networks_name;
    static const char * const networks_name;

    SpecParser() {};
    std::shared_ptr<Spec> parse(const std::string & filepath);
};

}

#endif /* end of include guard: SPEC_PARSER_H_ */