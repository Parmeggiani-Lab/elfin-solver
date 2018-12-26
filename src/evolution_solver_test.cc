#include "evolution_solver.h"

#include "test_data.h"
#include "test_stat.h"
#include "input_manager.h"
#include "basic_node_generator.h"

namespace elfin {

TestStat EvolutionSolver::test() {
    TestStat ts;

    // Test solving free body type work area.
    InputManager::load_test_config(
        /*spec_file=*/ "examples/quarter_snake_free_far.json");

    EvolutionSolver solver;

    // Test solver runs without crashing.
    {
        ts.tests++;

        LogLvl original_ll = ::get_log_level();

        ::set_log_level(LOG_RAW);
        solver.run();
        ::set_log_level(original_ll);
    }

    // Test that expected solution was obtained.
    {
        ts.tests++;
        size_t const n_work_areas = SPEC.work_areas().size();
        panic_if(n_work_areas != 1,
                 "Expected spec to have exactly 1 work area, but there is %lu\n",
                 n_work_areas);

        auto const& work_area = begin(SPEC.work_areas())->second;

        std::vector<NodeTeamSP> const& solutions =
            solver.best_sols().at(work_area->name());
        size_t const n_solutions = solutions.size();
        panic_if(n_solutions != OPTIONS.keep_n,
                 "Expected %lu solutions, but there is %lu\n",
                 OPTIONS.keep_n, n_solutions);

        auto best_team = solutions.at(0);
        auto& nodes = best_team->nodes();
        auto& free_chains = best_team->free_chains();

        panic_if(free_chains.size() != 2,
                 "Expected %lu free chains, but there is %lu\n",
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
                err("Solver best solution differs from expectation.\n");

                err("Solver solution:\n");
                BasicNodeGenerator err_node_gen(tip_fc.node_sp());
                while (not err_node_gen.is_done()) {
                    raw_at(LOG_ERROR, "%s\n",
                           err_node_gen.next()->prototype_->name.c_str());
                }

                err("Expected solution:\n");
                for (auto& step : recipe) {
                    raw_at(LOG_ERROR, "%s\n",
                           step.mod_name.c_str());
                }

                break;
            }
            ++step_itr;
        }
    }

    return ts;
}

}  /* elfin */