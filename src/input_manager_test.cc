#include "input_manager.h"

#include "test_stat.h"
#include "test_consts.h"


namespace elfin {
    
/* tests data */

/* tests */
TestStat InputManager::test() {
    TestStat ts;

    load_test_input();

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