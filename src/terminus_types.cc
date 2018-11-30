#include "terminus_type.h"

namespace elfin {

void death_by_bad_terminus(std::string func_name, TerminusType term) {
    die(("%s received bad TerminusType::%s\n"),
        func_name.c_str(),
        TerminusTypeNames[term]);
}

}  /* elfin */