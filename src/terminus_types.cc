#include "terminus_type.h"

#include "random_utils.h"

namespace elfin {

void death_by_bad_terminus(std::string func_name, const TerminusType term) {
    die(("%s received bad TerminusType::%s\n"),
        func_name.c_str(),
        TerminusTypeNames[term]);
}

TerminusType random_term() {
    return get_dice(2) == 0 ? TerminusType::N : TerminusType::C;
}

}  /* elfin */