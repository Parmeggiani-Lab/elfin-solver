#include "work_area.h"

#include "test_stat.h"
#include "input_manager.h"

namespace elfin {

TestStat WorkArea::test() {
    TestStat ts;

    // Test invalid hinge rejection for 2H.
    {
        ts.tests++;

        InputManager::setup_test({
            "--spec_file",
            "examples/H_2h_invalid_hinge.json"
        });

        try {
            Spec const spec(OPTIONS);

            // Should throw InvalidHinge by now
            JUtil.error("Invalid hinge failed to trigger exception.\n");
            ts.errors++;
        }
        catch (InvalidHinge const& e) {
            // All good!
        }
    }

    // Test valid hinge acceptance for 2H.
    {
        ts.tests++;

        InputManager::setup_test({
            "--spec_file",
            "examples/H_2h.json"
        });

        try {
            Spec const spec(OPTIONS);
        }
        catch (InvalidHinge const& e) {
            // Should NOT throw InvalidHinge by now
            JUtil.error("Valid hinge triggered exception. Details: %s\n",
                        e.what());
            ts.errors++;
        }
    }

    // Test shared hinge decimation.
    {
        ts.tests++;

        InputManager::setup_test({
            "--spec_file",
            "examples/half_snake_2x1h_shared_hinge.json"
        });
        Spec const spec(OPTIONS);

        TRACE_NOMSG(spec.work_packages().size() != 1);
        auto const& wp = *begin(spec.work_packages());

        auto const& was = wp->work_area_keys();
        if (was.size() != 2) {
            ts.errors++;
            JUtil.error("Failed to decimate Path Guide network. "
                        "Expected 2 but got %zu work areas.\n", was.size());
        };
    }

    return ts;
}

}  /* elfin */