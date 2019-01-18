#include "hinge_team.h"

#include "test_data.h"
#include "test_stat.h"
#include "input_manager.h"
#include "path_generator.h"
#include "scoring.h"

namespace elfin {

/* tests */
TestStat HingeTeam::test() {
    TestStat ts;

    // Construction test.
    {
        InputManager::setup_test({
            "--spec_file",
            "examples/H_1h.json"
        });
        Spec const spec(OPTIONS);

        TRACE_NOMSG(spec.work_packages().size() != 1);
        auto& wp = *begin(spec.work_packages());

        TRACE_NOMSG(wp->n_work_areas() != 1);
        auto& wv = wp->work_areas();

        TRACE_NOMSG(wv.size() != 1);
        auto& wa = wv.at(0);

        // Initialize HingeTeam, copy hinge transform, build from recipe than
        // apply transform.
        HingeTeam team(wa.get(), OPTIONS.seed);

        // Make a copy of the tx because it's gonna get clear()'ed.
        auto hinge_tx = team.hinge_->tx_;
        team.implement_recipe(tests::H_1H_RECIPE, hinge_tx);

        ts.tests++;
        float const score = team.score();
        if (score > scoring::SCORE_FLOOR) {
            ts.errors++;
            JUtil.error("HingeTeam construction test failed.\n"
                        "Expected score 0\nGot score: %f\n",
                        score);

            std::ostringstream const_oss;
            const_oss << "Constructed nodes:\n";

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
    }

    return ts;
}

}  /* elfin */