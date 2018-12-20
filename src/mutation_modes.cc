#include "mutation_modes.h"

#include <algorithm>

namespace elfin {

MutationModeList gen_mutation_mode_list() {
    // NONE is excluded, hence the -1 and pre-increment of int_mode
    Vector<MutationMode> res(
        static_cast<int>(MutationMode::_ENUM_SIZE) - 1);

    int int_mode = static_cast<int>(MutationMode::NONE);
    std::generate(begin(res), end(res), [&] {
        return static_cast<MutationMode>(++int_mode);
    });

    return res;
}

MutationCounter gen_mutation_counter() {
    // NONE is excluded, hence the < and pre-increment of int_mode
    MutationCounter res;

    int int_mode = static_cast<int>(MutationMode::NONE);
    while (int_mode < static_cast<int>(MutationMode::_ENUM_SIZE)) {
        res[static_cast<MutationMode>(++int_mode)] = 0;
    }

    return res;
}

}  /* elfin */