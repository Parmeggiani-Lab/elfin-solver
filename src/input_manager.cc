#include "input_manager.h"

#include "arg_parser.h"
#include "random_utils.h"

namespace elfin {

Options const& OPTIONS =
    InputManager::options();
Cutoffs const& CUTOFFS =
    InputManager::cutoffs();
Database const& XDB =
    InputManager::xdb();
Spec const& SPEC =
    InputManager::spec();

GATimes const& GA_TIMES =
    InputManager::ga_times();

/* protected */
void InputManager::setup_cutoffs() {
    Cutoffs& cutoffs = instance().cutoffs_;

    cutoffs = {}; // zero struct

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
void InputManager::setup(int const argc, const char ** argv) {
    // Parse arguments into options struct
    instance().options_ = ArgParser(argc, argv).get_options();

    // Create output dir if not exists
    mkdir_ifn_exists(OPTIONS.output_dir.c_str());

    // Always report seed
    msg("Using master seed: %d\n", OPTIONS.rand_seed);

    // Setup data members
    setup_cutoffs();

    instance().xdb_.parse_from_json(parse_json(OPTIONS.xdb));

    msg("Using input file: %s\n", OPTIONS.input_file.c_str());
    instance().spec_.parse_from_json(parse_json(OPTIONS.input_file));
}

/* tests */
void InputManager::load_test_input() {
    msg("Loading test input\n");

    char const* argv[] = {
        "elfin", /* binary name */
        "--input_file", "examples/quarter_snake_free.json",
        "--xdb", "xdb.json",
        "--output_dir", "./output/",
        "--rand_seed", "0xbeef1337",
        "--len_dev_alw", "1",
        "--keep_n", "1",
        "--ga_pop_size", "1024",
        "--ga_iters", "100",
        "--ga_survive_rate", "0.02",
        "--ga_stop_stagnancy", "10"
    };

    size_t const argc = sizeof(argv) / sizeof(argv[0]);
    InputManager::setup(argc, argv);
}

}  /* elfin */