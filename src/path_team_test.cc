#include "path_team.h"

#include "test_data.h"
#include "test_stat.h"
#include "input_manager.h"

namespace elfin {

/* tests data */
PathTeam PathTeam::build_team(tests::StepList const& steps) {
    // Create team
    TRACE_PANIC(SPEC.work_areas().size() != 1);
    auto& wa = begin(SPEC.work_areas())->second;
    PathTeam team(wa.get());

    if (not steps.empty()) {
        std::string const& first_mod_name = steps[0].mod_name;
        auto last_node = team.add_member(XDB.get_module(first_mod_name));

        for (auto itr = begin(steps); itr < end(steps) - 1; ++itr) {
            auto const& step = *itr;

            // last_node has already been created. Now create next node.

            auto src_mod = XDB.get_module(step.mod_name);
            auto dst_mod = XDB.get_module((itr + 1)->mod_name);
            TRACE_PANIC(not src_mod);
            TRACE_PANIC(not dst_mod);

            size_t const src_chain_id = src_mod->find_chain_id(step.src_chain);
            size_t const dst_chain_id = dst_mod->find_chain_id(step.dst_chain);

            // Find ProtoLink
            auto pt_link = src_mod->find_link_to(
                               src_chain_id,
                               step.src_term,
                               dst_mod,
                               dst_chain_id);

            // Find FreeChain
            TRACE_PANIC(team.free_chains_.size() != 2);
            auto fc_itr = std::find_if(
                              begin(team.free_chains_),
                              end(team.free_chains_),
            [&](auto const & fc) { return fc.term == step.src_term; });

            TRACE_PANIC(fc_itr == end(team.free_chains_));
            FreeChain const& free_chain = *fc_itr;

            last_node = team.grow_tip(free_chain, pt_link);
        }
    }

    return team;
}

/* tests */
TestStat PathTeam::test() {
    TestStat ts;

    // Construction test.
    {
        auto team = build_team(tests::quarter_snake_free_recipe);
        TRACE_PANIC(team.free_chains_.size() != 2);

        V3fList team_points = team.collect_points(team.free_chains_[0].node);

        ts.tests++;
        float const score = team.calc_score();
        if (score > 1e-6) {
            ts.errors++;
            JUtil.error("Construction test failed.\n"
                "Expected score 0\nGot score: %f\n", score);
        }

    }


    return ts;
}

}  /* elfin */