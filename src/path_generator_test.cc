#include "path_generator.h"

#include "test_stat.h"
#include "test_data.h"
#include "input_manager.h"
#include "path_team.h"

namespace elfin {

TestStat PathGenerator::test() {
    TestStat ts;

    // Test points gathered are of the correct size.
    {
        ts.tests++;

        InputManager::setup_test({
            "--spec_file",
            "examples/quarter_snake_free.json"
        });
        Spec const spec(OPTIONS);

        // Need to repeat part of construction test. Therefore make sure
        // PathTeam::test() is cleared before this test is called.
        TRACE_NOMSG(spec.work_packages().size() != 1);
        auto& wp = *begin(spec.work_packages());

        TRACE_NOMSG(wp->det_verse_size() != 1);
        auto& wv = wp->det_verse();

        TRACE_NOMSG(wv.size() != 1);
        auto& wa = wv.at(0);

        PathTeam team(wa.get());
        team.implement_recipe(tests::QUARTER_SNAKE_FREE_RECIPE);

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