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
void InputManager::parse_options(int const argc, const char ** argv) {
    // Parse arguments into options struct
    instance().options_ = ArgParser(argc, argv).get_options();

    // Create output dir if not exists
    JUtil.mkdir_ifn_exists(OPTIONS.output_dir.c_str());

    // Setup data members
    setup_cutoffs();
}

void InputManager::setup() {
    instance().xdb_.parse_from_json(parse_json(OPTIONS.xdb));

    JUtil.info("Using spec file: %s\n", OPTIONS.spec_file.c_str());
    instance().spec_.parse_from_json(parse_json(OPTIONS.spec_file));

    JUtil.info("Spec has %lu fixed areas and %lu work areas\n",
        SPEC.fixed_areas().size(), SPEC.work_areas().size());
}

}  /* elfin */