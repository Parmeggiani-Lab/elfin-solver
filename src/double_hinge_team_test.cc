#include "double_hinge_team.h"

#include "test_data.h"
#include "test_stat.h"
#include "input_manager.h"
#include "path_generator.h"

namespace elfin {

TestStat DoubleHingeTeam::test() {
    TestStat ts;

    // Construction test.
    {
        InputManager::load_test_config("examples/H_2h.json");

        TRACE_NOMSG(SPEC.work_areas().size() != 1);
        auto& [wa_name, wa] = *begin(SPEC.work_areas());

        // Initialize DoubleHingeTeam, copy hinge transform, build from recipe than
        // apply transform.
        DoubleHingeTeam team(wa.get());

        // Make a copy of the tx because it's gonna get clear()'ed.
        auto const& recipe = tests::H_2H_RECIPE;
        auto const& omap = wa->occupant_map;

        auto om_itr = omap.find(recipe[0].ui_name);
        TRACE_NOMSG(om_itr == end(omap));

        auto const& [hinge_name, hinge_ui_joint_key] = *om_itr;
        
        // Do not use UIJointKey's tx because it doesn't have the correct
        // rotation. Must use UIModuleKey
        auto hinge_ui_module_key = hinge_ui_joint_key->occupant.ui_module;
        auto const& hinge_tx = hinge_ui_module_key->tx;

        team.implement_recipe(recipe, hinge_tx);

        ts.tests++;
        if (team.score() > 1e-6) {
            ts.errors++;
            JUtil.error("DoubleHingeTeam construction test failed.\n"
                        "Expected score 0\nGot score: %f\n",
                        team.score());

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