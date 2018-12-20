#include "basic_node_team.h"

#include "test_consts.h"
#include "test_stat.h"
#include "input_manager.h"


namespace elfin {

/* tests data */
struct Step {
    std::string name = "not initialized";
    TerminusType out_term = TerminusType::NONE;
    size_t chain_id = 9999;
};

typedef std::vector<Step> StepList;

StepList const quarter_snake_free_recipe {
    {"D79_aC2_04", TerminusType::C, 1},
    {"D79", TerminusType::C, 0},
    {"D79", TerminusType::C, 0},
    {"D79", TerminusType::C, 0},
    {"D79_j1_D54", TerminusType::C, 0},
    {"D54_j1_D79", TerminusType::C, 0},
    {"D79_j2_D14", TerminusType::C, 0},
};

BasicNodeTeam build_team(StepList const& steps) {
    // Create team
    NICE_PANIC(SPEC.work_areas().size() != 1);
    auto& wa = begin(SPEC.work_areas())->second;
    BasicNodeTeam team(wa.get());

    for (Step const& step : steps) {

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