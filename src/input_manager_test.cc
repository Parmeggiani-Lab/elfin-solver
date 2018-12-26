#include "input_manager.h"

#include "test_stat.h"
#include "test_consts.h"


namespace elfin {

void InputManager::load_unit_test_config(std::string const& spec_file) {
    msg("Loading test input\n");

    char const* argv[] = {
        "elfin", /* binary name */
        "--config_file", "config/unit_test.json",
        "--spec_file", spec_file.c_str()
    };

    size_t const argc = sizeof(argv) / sizeof(argv[0]);
    InputManager::setup(argc, argv);
}

/* tests */
TestStat InputManager::test() {
    TestStat ts;

    load_unit_test_config();

    // Test spec parsing.
    ts.tests++;
    if (SPEC.work_areas().size() != 1) {
        ts.errors++;
        err("Spec parsing should get 1 work area but got %zu\n",
            SPEC.work_areas().size());
    }
    else {
        auto& wa = begin(SPEC.work_areas())->second; // unique_ptr

        // Test parsed points.
        V3fList const& points_test = wa->points();

        ts.tests++;
        if (quarter_snake_free_coordinates != points_test) {
            ts.errors++;
            err("Work area point parsing test failed\n");
            err("Expected:\n");
            for (auto& p : quarter_snake_free_coordinates) {
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