#include "path_team.h"

#include "test_data.h"
#include "test_stat.h"
#include "input_manager.h"

namespace elfin {

/* tests */
TestStat PathTeam::test() {
    TestStat ts;

    InputManager::load_test_config(
        "examples/quarter_snake_free.json");

    // Construction test.
    {
        TRACE_NOMSG(SPEC.work_areas().size() != 1);
        auto& [wa_name, wa] = *begin(SPEC.work_areas());
        PathTeam team(wa.get(), tests::quarter_snake_free_recipe);
        team.calc_score();

        TRACE_NOMSG(team.free_chains_.size() != 2);

        ts.tests++;
        if (team.score() > 1e-6) {
            ts.errors++;
            JUtil.error("PathTeam construction test failed.\n"
                        "Expected score 0\nGot score: %f\n",
                        team.score());
        }

    }

    // Mutation test.
    {
        JUtil.warn("TODO: Mutation test\n");
    }

    return ts;
}

}  /* elfin */