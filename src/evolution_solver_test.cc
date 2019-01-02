#include "evolution_solver.h"

#include "test_data.h"
#include "test_stat.h"
#include "input_manager.h"
#include "path_team.h"
#include "path_generator.h"

namespace elfin {

TestStat EvolutionSolver::test() {
    TestStat ts;

    // Test expected solution.
    auto test_fragment =
        [&](std::string const & spec, tests::Recipe const & recipe)
    {
        ts.tests++;

        // Force 1 thread for reproduciblility.
        InputManager::load_test_config(
            /*spec_file=*/ spec,
            /*n_workers=*/ 1);


        JUtilLogLvl original_ll = JUtil.get_log_lvl();

        JUtil.set_log_lvl(LOGLVL_WARNING);
        EvolutionSolver solver;
        solver.run();
        JUtil.set_log_lvl(original_ll);

        auto const& [work_area_name, work_area] = *begin(SPEC.work_areas());
        auto const& solutions = solver.best_sols(work_area_name);  // TeamPtrMaxHeap

        try { // Catch bad_cast
            auto best_pt =
                static_cast<PathTeam const&>(*solutions.top());
            auto path_gen = best_pt.gen_path();

            // Might need to reverse recipe because data exported from
            // elfin-ui does not gurantee consistent starting tip.
            bool const should_reverse = recipe[0].mod_name !=
                                        path_gen.peek()->prototype_->name;

            auto recipe_fwd = should_reverse ?
                              tests::Recipe(rbegin(recipe), rend(recipe)) :
                              recipe;
            auto step_itr = begin(recipe_fwd);

            while (not path_gen.is_done()) {
                auto node = path_gen.next();
                if (step_itr == end(recipe_fwd) or
                        step_itr->mod_name != node->prototype_->name) {
                    ts.errors++;
                    JUtil.error("Solver best solution differs from expectation.\n");

                    std::ostringstream sol_oss;
                    sol_oss << "Solver solution:\n";

                    auto team_pg = best_pt.gen_path();
                    while (not team_pg.is_done()) {
                        sol_oss << team_pg.next()->prototype_->name << "\n";
                    }
                    JUtil.error(sol_oss.str().c_str());

                    std::ostringstream exp_oss;
                    exp_oss << "Expected solution:\n";
                    for (auto& step : recipe_fwd) {
                        exp_oss << step.mod_name << "\n";
                    }

                    JUtil.error(exp_oss.str().c_str());
                    break;
                }
                ++step_itr;
            }
        }
        catch (std::bad_cast const& exp) {
            ts.errors++;
            JUtil.error("Bad cast in %s\n", __PRETTY_FUNCTION__);
        }
    };

    // Test short free type work area.
    test_fragment("examples/quarter_snake_free.json",
                  tests::quarter_snake_free_recipe);

    // Test medium free type work area.
    test_fragment("examples/half_snake_free.json",
                  tests::half_snake_free_recipe);

    // Test short 1H type work area.
    test_fragment("examples/quarter_snake_1h.json",
                  tests::quarter_snake_free_recipe);

    // Test medium 1H type work area.
    test_fragment("examples/half_snake_1h.json",
                  tests::half_snake_free_recipe);

    // Test 2x medium 1H type work area.
    test_fragment("examples/half_snake_2x1h.json",
                  tests::half_snake_free_recipe);

    // Test short 2H type work area.
    // test_fragment("examples/H_2h.json",
    //               tests::H_2h_recipe);

    return ts;
}

}  /* elfin */