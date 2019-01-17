#include "path_team.h"

#include "test_data.h"
#include "test_stat.h"
#include "input_manager.h"
#include "path_generator.h"
#include "scoring.h"

namespace elfin {

/* tests */
TestStat PathTeam::test() {
    TestStat ts;

    auto construction_test =
    [&](std::string const & spec_file, tests::Recipe const & recipe) {
        // Load the spec file so we have a criterion to score against.
        InputManager::setup_test({"--spec_file", spec_file});
        Spec const spec(OPTIONS);

        TRACE_NOMSG(spec.work_packages().size() != 1);
        auto& [wp_name, wp] = *begin(spec.work_packages());

        TRACE_NOMSG(wp->work_areas().size() != 1);
        auto& [wa_name, wa] = *begin(wp->work_areas());

        PathTeam team(wa.get());
        team.implement_recipe(recipe);

        size_t const n_free_terms = team.free_terms_.size();
        TRACE(n_free_terms != 2,
              "%zu\n", n_free_terms);

        ts.tests++;
        float const score = team.score();
        if (not scoring::almost_eq(score, 0)) {
            ts.errors++;
            JUtil.error("PathTeam construction test of %s failed.\n"
                        "Expected score 0\nGot score: %f\n",
                        spec_file.c_str(),
                        score);

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
            inp_oss << "Spec module point:\n";
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

    // Checksum test.
    {
        ts.tests++;

        PathTeam team(nullptr);

        // Don't call the public implement_recipe() because that in turn calls
        // calc_score(), which requires non nullptr work_area_.
        team.virtual_implement_recipe(tests::H_1H_RECIPE,
                                      FirstLastNodeKeyCallback(),
                                      Transform());
        team.calc_checksum();
        auto const cksm1 = team.checksum();

        team.virtual_implement_recipe(tests::H_1H_CHAIN_CHANGED_RECIPE,
                                      FirstLastNodeKeyCallback(),
                                      Transform());
        team.calc_checksum();
        auto const cksm2 = team.checksum();

        if (cksm1 == cksm2) {
            ts.errors++;
            JUtil.error("PathTeam checksum test failed. 0x%x vs 0x%x\n",
                        cksm1, cksm2);
        }
    }

    return ts;
}

}  /* elfin */