#include "hinge_team.h"
#include "test_data.h"
#include "test_stat.h"
#include "input_manager.h"
#include "path_generator.h"

namespace elfin {

/* tests */
TestStat HingeTeam::test() {
    TestStat ts;

    InputManager::load_test_config("examples/H_1h.json");

    // Construction test.
    {
        TRACE_NOMSG(SPEC.work_areas().size() != 1);
        auto& [wa_name, wa] = *begin(SPEC.work_areas());

        // Initialize HingeTeam, copy hinge transform, build from recipe than
        // apply transform.
        HingeTeam team(wa.get());
        auto const hinge_tx = team.hinge_->tx_;
        team.from_recipe(tests::H_1h_recipe);
        for (auto& node_itr : team.nodes_) {
            node_itr.second->tx_  = hinge_tx * node_itr.second->tx_;
        }

        ts.tests++;
        if (team.score() > 1e-6) {
            ts.errors++;
            JUtil.error("HingeTeam construction test failed.\n"
                        "Expected score 0\nGot score: %f\n",
                        team.score());

            auto const& const_points =
                team.gen_path().collect_points();
            std::ostringstream const_oss;
            const_oss << "Constructed nodes:\n";

            auto team_pg = team.gen_path();
            while (not team_pg.is_done()) {
                const_oss << *team_pg.next() << "\n";
            }
            JUtil.error(const_oss.str().c_str());

            std::ostringstream inp_oss;
            inp_oss << "Input modules:\n";
            for (auto& p : wa->points) {
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