#include "hinge_team.h"
#include "test_data.h"
#include "test_stat.h"
#include "input_manager.h"

namespace elfin {

/* tests */
TestStat HingeTeam::test() {
    TestStat ts;

    InputManager::load_test_config(
        "examples/H_1h.json");

    // Construction test.
    {
        TRACE_NOMSG(SPEC.work_areas().size() != 1);
        auto& wa = begin(SPEC.work_areas())->second;
        HingeTeam team(wa.get(), tests::H_1h_recipe);
        team.calc_score();

        TRACE_NOMSG(team.free_chains_.size() != 2);

        ts.tests++;
        if (team.score() > 1e-6) {
            ts.errors++;
            JUtil.error("HingeTeam construction test failed.\n"
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