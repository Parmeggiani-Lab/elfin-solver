#include "mutation.h"

#include <vector>
#include <unordered_set>
#include <algorithm>

namespace elfin {

namespace mutation {

std::unordered_set<Mode> const disabled_modes = {
    // Mode::ERODE,
    // Mode::DELETE,
    // Mode::INSERT,
    // Mode::SWAP,
    // Mode::CROSS,
    // Mode::REGENERATE
};

ModeList gen_mode_list() {
    // NONE is excluded, hence the -1 and pre-increment of int_mode
    std::vector<Mode> res(
        static_cast<int>(Mode::_ENUM_SIZE) - 1 - disabled_modes.size());

    int int_mode = static_cast<int>(Mode::NONE);
    std::generate(begin(res), end(res), [&] {
        Mode mode;
        while (0 < disabled_modes.count(mode = static_cast<Mode>(++int_mode))) {}
        return mode;
    });

    return res;
}

Counter gen_counter() {
    // NONE is excluded, hence the < and pre-increment of int_mode
    Counter res;

    int int_mode = static_cast<int>(Mode::NONE);
    while (int_mode < static_cast<int>(Mode::_ENUM_SIZE)) {
        res[static_cast<Mode>(++int_mode)] = 0;
    }

    return res;
}

}  /* mutation */

}  /* elfin */