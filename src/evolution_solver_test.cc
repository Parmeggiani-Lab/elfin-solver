#include "evolution_solver.h"

#include "test_data.h"
#include "test_stat.h"
#include "input_manager.h"
#include "basic_node_generator.h"
#include "basic_node_team.h"

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
        JUtil.panic_if(n_work_areas != 1,
                       "Expected spec to have exactly 1 work area, but there is %zu\n",
                       n_work_areas);

        auto const& work_area = begin(SPEC.work_areas())->second;

        auto& solutions =
            solver.best_sols().at(work_area->name());
        size_t const n_solutions = solutions.size();
        JUtil.panic_if(n_solutions != OPTIONS.keep_n,
                       "Expected %zu solutions, but there is %zu\n",
                       OPTIONS.keep_n, n_solutions);

        try { // Catch bad_cast
            auto best_bnt =
                static_cast<BasicNodeTeam const&>(*solutions.at(0));
            auto& free_chains = best_bnt.free_chains();

            JUtil.panic_if(free_chains.size() != 2,
                           "Expected %zu free chains, but there is %zu\n",
                           2, free_chains.size());

            auto& recipe = tests::quarter_snake_free_recipe;
            std::string const& starting_name = recipe[0].mod_name;
            auto& tip_fc = free_chains[0].node_sp()->prototype_->name == starting_name ?
                           free_chains[0] : free_chains[1];

            auto step_itr = begin(recipe);
            BasicNodeGenerator node_gen(tip_fc.node_sp());
            while (not node_gen.is_done()) {
                auto node = node_gen.next();
                if (step_itr == end(recipe) or
                        step_itr->mod_name != node->prototype_->name) {
                    ts.errors++;
                    JUtil.error("Solver best solution differs from expectation.\n");

                    std::ostringstream oss;
                    oss << "Solver solution:\n";

                    BasicNodeGenerator err_node_gen(tip_fc.node_sp());
                    while (not err_node_gen.is_done()) {
                        oss << err_node_gen.next()->prototype_->name << "\n";
                    }

                    JUtil.error("Expected solution:\n");
                    for (auto& step : recipe) {
                        oss << step.mod_name << "\n";
                    }

                    JUtil.error(oss.str().c_str());
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