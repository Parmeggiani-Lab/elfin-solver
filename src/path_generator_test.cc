#include "path_generator.h"

#include "test_stat.h"
#include "test_data.h"
#include "input_manager.h"
#include "path_team.h"

namespace elfin {

TestStat PathGenerator::test() {
    TestStat ts;

    InputManager::load_test_config(
        "examples/quarter_snake_free.json");

    // Test points gathered are of the correct size.
    {
        ts.tests++;

        // Need to repeat part of construction test. Therefore make sure
        // PathTeam::test() is cleared before this test is called.
        TRACE_NOMSG(SPEC.work_areas().size() != 1);
        auto& [wa_name, wa] = *begin(SPEC.work_areas());
        PathTeam team(wa.get());
        team.from_recipe(tests::quarter_snake_free_recipe);

        auto const& points = team.gen_path().collect_points();
        if (points.size() != team.size()) {
            ts.errors++;
            JUtil.error("PathGenerator collect points failed.\n"
                        "points.size()=%zu, size()=%zu\n",
                        points.size(),
                        team.size());
        }
    }

    return ts;
}

}  /* elfin */