#include "mutation_modes.h"

#include <algorithm>

namespace elfin {

MutationModeList gen_mutation_mode_list() {
    Vector<MutationMode> res(MutationMode::_ENUM_SIZE_PLUS_ONE - 1);

    int int_mode = MutationMode::_ENUM_START;
    std::generate(res.begin(), res.end(), [&] {
        return static_cast<MutationMode>(++int_mode);
    });
    
    return res;
}

MutationCounter gen_mutation_counter() {
    MutationCounter res;

    int int_mode = MutationMode::_ENUM_START;
    while (int_mode < MutationMode::_ENUM_SIZE_PLUS_ONE) {
        res[static_cast<MutationMode>(++int_mode)] = 0;
    }

    return res;
}

}  /* elfin */