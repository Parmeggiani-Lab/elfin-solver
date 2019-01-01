#include "evolution_solver.h"

#include "test_data.h"
#include "test_stat.h"
#include "input_manager.h"
#include "path_team.h"
#include "path_generator.h"

namespace elfin {

TestStat EvolutionSolver::test() {
    TestStat ts;

    // Test solving free body type work area.
    // Force 1 thread for reproduciblility.
    InputManager::load_test_config(
        /*spec_file=*/ "examples/quarter_snake_free_far.json",
        /*n_workers=*/ 1);

    EvolutionSolver solver;
    // Test solver runs without crashing.
    {
        ts.tests++;

        JUtilLogLvl original_ll = JUtil.get_log_lvl();

        JUtil.set_log_lvl(LOGLVL_WARNING);
        solver.run();
        JUtil.set_log_lvl(original_ll);
    }

    // Test that expected solution was obtained.
    {
        ts.tests++;
        size_t const n_work_areas = SPEC.work_areas().size();
        PANIC_IF(n_work_areas != 1,
                 "Expected spec to have exactly 1 work area, but there is %zu\n",
                 n_work_areas);

        auto const& [work_area_name, work_area] = *begin(SPEC.work_areas());
        auto& solutions = solver.best_sols().at(work_area_name);
        size_t const n_solutions = solutions.size();
        PANIC_IF(n_solutions != OPTIONS.keep_n,
                 "Expected %zu solutions, but there is %zu\n",
                 OPTIONS.keep_n, n_solutions);

        try { // Catch bad_cast
            auto best_pt =
                static_cast<PathTeam const&>(*solutions.at(0));
            auto path_gen = best_pt.gen_path();

            // Might need to reverse recipe because data exported from
            // elfin-ui does not gurantee consistent starting tip.
            bool const should_reverse =
                tests::quarter_snake_free_recipe[0].mod_name !=
                path_gen.peek()->prototype_->name;

            auto recipe = should_reverse ?
                          tests::Recipe(rbegin(tests::quarter_snake_free_recipe),
                            rend(tests::quarter_snake_free_recipe)) :
                          tests::quarter_snake_free_recipe;
            auto step_itr = begin(recipe);

            while (not path_gen.is_done()) {
                auto node = path_gen.next();
                if (step_itr == end(recipe) or
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
                    for (auto& step : recipe) {
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
    }

    return ts;
}

}  /* elfin */