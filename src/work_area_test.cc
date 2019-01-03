#include "work_area.h"

#include "test_stat.h"
#include "input_manager.h"

namespace elfin {

TestStat WorkArea::test() {
    TestStat ts;

    // Test invalid hinge rejection for 2H.
    {
        ts.tests++;

        try {

            InputManager::setup_test({
                "--spec_file",
                "examples/H_2h_invalid_hinge.json"
            });

            // Should throw InvalidHingeException by now
            JUtil.error("Invalid hinge failed to trigger exception.\n");
            ts.errors++;
        }
        catch (InvalidHingeException const& e) {
            // All good!
        }
    }

    // Test valid hinge acceptance for 2H.
    {
        ts.tests++;

        try {
            InputManager::setup_test({
                "--spec_file",
                "examples/H_2h.json"
            });
        }
        catch (InvalidHingeException const& e) {
            // Should NOT throw InvalidHingeException by now
            JUtil.error("Valid hinge triggered exception. Details: %s\n",
                e.what());
            ts.errors++;
        }
    }

    return ts;
}

}  /* elfin */