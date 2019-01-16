#include "input_manager.h"

#include "arg_parser.h"
#include "random_utils.h"

namespace elfin {

Options const& OPTIONS =
    InputManager::options();
Cutoffs const& CUTOFFS =
    InputManager::cutoffs();
Database const& XDB =
    InputManager::mutable_xdb();

GATimes const& GA_TIMES =
    InputManager::ga_times();

/* protected */
void InputManager::setup_cutoffs() {
    Cutoffs& cutoffs = instance().cutoffs_;

    cutoffs = {};  // zero struct

    cutoffs.pop_size =
        OPTIONS.ga_pop_size;

    // Force survivors > 0
    cutoffs.survivors =
        std::max((size_t) 1,
                 (size_t) std::round(OPTIONS.ga_survive_rate * OPTIONS.ga_pop_size));

    cutoffs.non_survivors =
        (OPTIONS.ga_pop_size - cutoffs.survivors);
}

/* public */
InputManager::Args const InputManager::test_args = {
    "elfin",  // Binary file name.
    "--config_file", "config/test_config.json"
};

void InputManager::parse(Args const& args, bool const skip_xdb) {
    std::vector<char const*> argv;
    for (auto const& arg : args) {
        argv.push_back(arg.c_str());
    }

    InputManager::parse(argv.size(), argv.data(), skip_xdb);
}

void InputManager::parse(int const argc, char const** const argv, bool const skip_xdb) {
    // Parse arguments into options struct.
    instance().options_ = ArgParser(argc, argv).get_options();

    // Create output dir if not exists.
    JUtil.mkdir_ifn_exists(OPTIONS.output_dir.c_str());

    if (not skip_xdb) {
        JUtil.error("Parse XDB\n");
        instance().xdb_.parse(OPTIONS);
    }

    // Setup data members.
    setup_cutoffs();

    parallel::init();
    random::init();
}

}  /* elfin */