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

TestStat InputManager::test() {
    TestStat ts;

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

    // Test spec parsing
    ts.tests++;
    if (SPEC.work_areas().size() != 1) {
        ts.errors++;
        err("Spec parsing should get 1 work area but got %lu\n",
            SPEC.work_areas().size());
    }
    else {
        auto const& kv = *begin(SPEC.work_areas());
        auto& wa = kv.second; // unique_ptr

        // Test parsed points
        V3fList points = {
            Vector3f(82.54196166992188, 3.187546730041504, -44.660125732421875),
            Vector3f(54.139976501464844, -23.924468994140625, -35.15853500366211),
            Vector3f(26.635669708251953, -57.53522872924805, -29.187021255493164),
            Vector3f(21.75318145751953, -63.43537139892578, -1.899409294128418),
            Vector3f(12.520357131958008, -50.98127365112305, 13.686529159545898),
            Vector3f(-4.097459316253662, -37.3050651550293, 18.167621612548828),
            Vector3f(-40.844879150390625, -42.66680908203125, 7.421332359313965)
        };
        V3fList points_test = wa->to_points();

        ts.tests++;
        if (points != points_test) {
            ts.errors++;
            err("Work area point parsing test failed\n");
            err("Expected:\n");
            for (auto& p : points) {
                raw_at(LOG_WARN, "%s\n", p.to_string().c_str());
            }
            err("But got:\n");
            for (auto& p : points_test) {
                raw_at(LOG_WARN, "%s\n", p.to_string().c_str());
            }
        }
    }

    return ts;
}

}  /* elfin */