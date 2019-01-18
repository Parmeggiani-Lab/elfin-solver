#include "double_hinge_team.h"

#include "test_data.h"
#include "test_stat.h"
#include "input_manager.h"
#include "path_generator.h"
#include "scoring.h"

namespace elfin {

TestStat DoubleHingeTeam::test() {
    TestStat ts;

    // Construction test.
    {
        InputManager::setup_test({
            "--spec_file", "examples/H_2h.json"
        });
        Spec const spec(OPTIONS);

        TRACE_NOMSG(spec.work_packages().size() != 1);
        auto& wp = *begin(spec.work_packages());

        TRACE_NOMSG(wp->n_work_area_keys() != 1);
        auto& wv = wp->work_area_keys();

        TRACE_NOMSG(wv.size() != 1);
        auto& wa = wv.at(0);

        // Initialize DoubleHingeTeam, copy hinge transform, build from recipe than
        // apply transform.
        DoubleHingeTeam team(wa, OPTIONS.seed);

        auto const& recipe = tests::H_2H_RECIPE;
        auto const& omap = wa->occupied_joints;

        auto om_itr = omap.find(recipe[0].ui_name);
        TRACE_NOMSG(om_itr == end(omap));

        auto const& [hinge_name, hinge_ui_joint_key] = *om_itr;

        // Do not use UIJointKey's tx because it doesn't have the correct
        // rotation. Must use UIModKey
        auto hinge_ui_module_key = hinge_ui_joint_key->occupant.ui_module;
        auto const& hinge_tx = hinge_ui_module_key->tx;

        team.implement_recipe(recipe, hinge_tx);

        ts.tests++;
        float const score = team.score();
        if (score > scoring::SCORE_FLOOR) {
            ts.errors++;
            JUtil.error("DoubleHingeTeam construction test failed.\n"
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