#include "path_team.h"

#include "test_data.h"
#include "test_stat.h"
#include "input_manager.h"
#include "path_generator.h"

namespace elfin {

/* tests */
TestStat PathTeam::test() {
    TestStat ts;

    auto construction_test =
    [&](std::string const & spec_file, tests::Recipe const & recipe) {
        InputManager::setup_test({"--spec_file", spec_file});

        auto& [wa_name, wa] = *begin(SPEC.work_areas());
        PathTeam team(wa.get());
        team.implement_recipe(recipe);

        TRACE_NOMSG(team.free_terms_.size() != 2);

        ts.tests++;
        if (team.score() > 1e-6) {
            ts.errors++;
            JUtil.error("PathTeam construction test of %s failed.\n"
                        "Expected score 0\nGot score: %f\n",
                        spec_file.c_str(),
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
            auto const& [ui_key, inp_points] = *begin(wa->path_map);
            inp_oss << "Input modules:\n";
            for (auto& p : inp_points) {
                inp_oss << p << "\n";
            }

            JUtil.error(inp_oss.str().c_str());
        }
    };

    // Short construction test.
    construction_test("examples/quarter_snake_free.json",
                      tests::QUARTER_SNAKE_FREE_RECIPE);

    // Long construction test.
    construction_test("examples/half_snake_free.json",
                      tests::HALF_SNAKE_FREE_RECIPE);

    // Mutation test.
    {
        JUtil.warn("TODO: PathTeam mutation operator tests\n");
    }

    return ts;
}

}  /* elfin */