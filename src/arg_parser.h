#ifndef ARG_PARSER_H_
#define ARG_PARSER_H_

#include "jutil.h"
#include "../data/TypeDefs.h"

namespace elfin {
    class ArgParser
    {
    private:

    public:
        OptionPack parse(const int argc, char const *argv[]) const;
        Points3f parse_input(const OptionPack & options) const;
    };
}

#endif /* end of include guard: ARG_PARSER_H_ */