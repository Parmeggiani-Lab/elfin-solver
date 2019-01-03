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

            // Should throw InvalidHingeException by now
            JUtil.error("Invalid hinge failed to trigger exception\n");
            ts.errors++;
        }
        catch (std::domain_error const& e) {

        }
    }

    return ts;
}

}  /* elfin */