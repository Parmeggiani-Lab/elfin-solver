#include "input_manager.h"

#include "test_stat.h"
#include "test_data.h"


namespace elfin {

void InputManager::load_test_config(std::string const& spec_file) {
    JUtil.info("Loading test config\n");

    char const* argv[] = {
        "elfin", /* binary name */
        "--config_file", "config/unit_test.json",
        "--spec_file", spec_file.c_str()
    };

    size_t const argc = sizeof(argv) / sizeof(argv[0]);
    InputManager::parse_options(argc, argv);
    InputManager::setup();
}

/* tests */
TestStat InputManager::test() {
    TestStat ts;

    load_test_config();

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
        if (tests::quarter_snake_free_coordinates != points_test) {
            ts.errors++;
            err("Work area point parsing test failed\n");
            std::ostringstream oss;
            oss << "Expected:\n";
            for (auto& p : tests::quarter_snake_free_coordinates) {
                oss << p.to_string() << "\n";
            }
            err("But got:\n");
            for (auto& p : points_test) {
                oss << p.to_string() << "\n";
            }

            err(oss.str().c_str());
        }
    }

    return ts;
}

}  /* elfin */