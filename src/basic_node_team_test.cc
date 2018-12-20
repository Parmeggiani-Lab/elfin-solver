#include "basic_node_team.h"

#include "test_consts.h"
#include "test_stat.h"
#include "input_manager.h"


namespace elfin {

/* tests data */
struct Step {
    std::string mod_name = "";
    TerminusType src_term = TerminusType::NONE;
    std::string src_chain = "";
    std::string dst_chain = "";
};

StepList const quarter_snake_free_recipe {
    {"D79_aC2_04", TerminusType::C, "B", "A"},
    {"D79", TerminusType::C, "A",  "A"},
    {"D79", TerminusType::C, "A",  "A"},
    {"D79", TerminusType::C, "A",  "A"},
    {"D79_j1_D54", TerminusType::C, "A",  "A"},
    {"D54_j1_D79", TerminusType::C, "A",  "A"},
    {"D79_j2_D14", TerminusType::C, "A",  "A"},
};

BasicNodeTeam BasicNodeTeam::build_team(StepList const& steps) {
    // Create team
    NICE_PANIC(SPEC.work_areas().size() != 1);
    auto& wa = begin(SPEC.work_areas())->second;
    BasicNodeTeam team(wa.get());

    if (not steps.empty()) {
        std::string const& first_mod_name = steps[0].mod_name;
        NodeSP last_node = team.add_member(XDB.get_module(first_mod_name));

        for (auto itr = begin(steps); itr < end(steps) - 1; ++itr) {
            Step const& step = *itr;

            // last_node has already been created. Now create next node.

            auto src_mod = XDB.get_module(step.mod_name);
            auto dst_mod = XDB.get_module((itr + 1)->mod_name);
            NICE_PANIC(src_mod == nullptr);
            NICE_PANIC(dst_mod == nullptr);

            size_t const src_chain_id = src_mod->find_chain_id(step.src_chain);
            size_t const dst_chain_id = dst_mod->find_chain_id(step.dst_chain);

            // Find ProtoLink
            auto pt_link = src_mod->find_link_to(
                               src_chain_id,
                               step.src_term,
                               dst_mod,
                               dst_chain_id);
            wrn("%s to %s\n", step.mod_name.c_str(), (itr + 1)->mod_name.c_str());
            wrn("Tx: %s", pt_link->tx_.to_string().c_str());

            // Find FreeChain
            NICE_PANIC(team.free_chains_.size() != 2);
            auto fc_itr = std::find_if(
                              begin(team.free_chains_),
                              end(team.free_chains_),
            [&](auto const & fc) { return fc.term == step.src_term; });

            NICE_PANIC(fc_itr == end(team.free_chains_));
            FreeChain const& free_chain = *fc_itr;

            last_node = team.grow_tip(free_chain, pt_link);;
        }
    }

    return team;
}

/* tests */
TestStat BasicNodeTeam::test() {
    TestStat ts;

    // Construction test.
    {
        auto team = build_team(quarter_snake_free_recipe);
        NICE_PANIC(team.free_chains_.size() != 2);

        V3fList team_points = team.collect_points(team.free_chains_[0].node_sp());

        wrn("Team points (from recipe):\n");
        for (Vector3f const& point : team_points) {
            wrn("%s\n", point.to_string().c_str());
        }

        ts.tests++;
        float const score = team.calc_score();
        if (score > 1e-6) {
            ts.errors++;
            err("Construction test failed.\n"
                "Expected score 0\nGot score: %f\n", score);
        }

    }


    return ts;
}

}  /* elfin */