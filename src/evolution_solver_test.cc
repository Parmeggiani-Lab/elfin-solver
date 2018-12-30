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

        // JUtil.set_log_lvl(LOGLVL_WARNING);
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

        auto const& work_area = begin(SPEC.work_areas())->second;

        auto& solutions =
            solver.best_sols().at(work_area->name());
        size_t const n_solutions = solutions.size();
        PANIC_IF(n_solutions != OPTIONS.keep_n,
                       "Expected %zu solutions, but there is %zu\n",
                       OPTIONS.keep_n, n_solutions);

        try { // Catch bad_cast
            auto best_bnt =
                static_cast<PathTeam const&>(*solutions.at(0));
            auto& free_chains = best_bnt.free_chains();

            PANIC_IF(free_chains.size() != 2,
                           "Expected %zu free chains, but there is %zu\n",
                           2, free_chains.size());

            auto& recipe = tests::quarter_snake_free_recipe;
            std::string const& starting_name = recipe[0].mod_name;

            auto fc_itr = begin(free_chains);
            auto& tip_fc = fc_itr->node->prototype_->name == starting_name ?
                           *fc_itr : *(++fc_itr);

            auto step_itr = begin(recipe);
            auto path_gen = tip_fc.node->gen_path();
            while (not path_gen.is_done()) {
                auto node = path_gen.next();
                if (step_itr == end(recipe) or
                        step_itr->mod_name != node->prototype_->name) {
                    ts.errors++;
                    JUtil.error("Solver best solution differs from expectation.\n");

                    std::ostringstream sol_oss;
                    sol_oss << "Solver solution:\n";

                    auto err_path_gen = tip_fc.node->gen_path();
                    while (not err_path_gen.is_done()) {
                        sol_oss << err_path_gen.next()->prototype_->name << "\n";
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