#include "path_team.h"

#include "test_data.h"
#include "test_stat.h"
#include "input_manager.h"
#include "path_generator.h"

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
        PathTeam team(wa.get());
        team.implement_recipe(tests::quarter_snake_free_recipe);

        TRACE_NOMSG(team.free_chains_.size() != 2);

        ts.tests++;
        if (team.score() > 1e-6) {
            ts.errors++;
            JUtil.error("PathTeam construction test failed.\n"
                        "Expected score 0\nGot score: %f\n",
                        team.score());

            auto const& const_points =
                team.gen_path().collect_points();
            std::ostringstream const_oss;
            const_oss << "Constructed points:\n";

            auto team_pg = team.gen_path();
            while (not team_pg.is_done()) {
                const_oss << *team_pg.next() << "\n";
            }
            JUtil.error(const_oss.str().c_str());

            std::ostringstream inp_oss;
            auto const& inp_points = wa->points;
            inp_oss << "Input modules:\n";
            for (auto& p : inp_points) {
                inp_oss << p << "\n";
            }

            JUtil.error(inp_oss.str().c_str());
        }
    }

    // Mutation test.
    {
        JUtil.warn("TODO: Mutation test\n");
    }

    return ts;
}

}  /* elfin */