#include "evolution_solver.h"

#include <cmath>
#include <sstream>
#include <algorithm>
#include <stdlib.h>

#include "jutil.h"
#include "input_manager.h"
#include "parallel_utils.h"

namespace elfin {

/* private */
struct EvolutionSolver::PImpl {
    /* data */
    bool score_satisfied_;
    bool should_restart_ga_;
    double start_time_in_us_;
    size_t const debug_pop_print_n_;
    Crc32 last_best_checksum_;
    size_t itr_id;
    size_t restart_id;
    size_t gen_id;
    double tot_gen_time;
    size_t stagnant_count;

    /* ctors */
    PImpl(size_t const debug_pop_print_n) :
        debug_pop_print_n_(debug_pop_print_n) {
        reset();
    }

    /* modifiers */
    void reset() {
        score_satisfied_ = false;
        should_restart_ga_ = false;
        start_time_in_us_ = 0;
        last_best_checksum_ = 0x0000;
        itr_id = 0;
        restart_id = 0;
        gen_id = 0;
        tot_gen_time = 0.0f;
        stagnant_count = 0;
    }

    void summarize_generation(Population const& pop,
                              double const gen_start_time,
                              TeamSPMaxHeap& output)
    {
        // Stat collection
        auto const& best_team = pop.front_buffer()->front();
        auto const& worst_team = pop.front_buffer()->back();

        float const best_score = best_team->score();
        Crc32 const& best_checksum = best_team->checksum();
        double const gen_time =
            (JUtil.get_timestamp_us() - gen_start_time) / 1e3;

        tot_gen_time += gen_time;

        // Print score stats.
        JUtil.info("GA Restart #%zu Generation #%zu Total Iteration #%zu\n"
                   "  -  [best %.2f (cksm:%x, %.2f/module), worst %.2f, %.0fms]\n",
                   restart_id,
                   gen_id,
                   itr_id,
                   best_score,
                   best_checksum,
                   best_score / best_team->size(),
                   worst_team->score(),
                   gen_time);

        // Compute stagnancy & check inverted scores.
        if (best_checksum == last_best_checksum_) {
            stagnant_count++;
        }
        else {
            stagnant_count = 0;
        }

        last_best_checksum_ = best_checksum;

        // Print timing stats.
        size_t const n_gens = gen_id + 1;
        JUtil.info("Avg Times: "
                   "[Evolve=%.0f, Score=%.0f, Rank=%.0f, "
                   "Select=%.0f, Gen=%.0f]\n",
                   (double) GA_TIMES.evolve_time / n_gens,
                   (double) GA_TIMES.score_time / n_gens,
                   (double) GA_TIMES.rank_time / n_gens,
                   (double) GA_TIMES.select_time / n_gens,
                   (double) tot_gen_time / n_gens);

        // Update best solutions.
        size_t const max_keep = std::min(OPTIONS.keep_n, OPTIONS.ga_pop_size);
        for (auto const& team : *pop.front_buffer()) {
            if (output.empty() or team->score() < output.top()->score()) {
                // Make sure checksum doesn't equal any existing solution.
                bool checksum_repeat = false;
                TeamSPMaxHeap output_tmp;
                while (!output.empty()) {
                    auto const& existing_team = output.top();
                    checksum_repeat |= existing_team->checksum() == team->checksum();
                    output_tmp.push(std::move(output.top_and_pop()));
                }

                while (!output_tmp.empty()) {
                    output.push(std::move(output_tmp.top_and_pop()));
                }

                if (checksum_repeat)
                    continue;

                auto team_clone = team->clone();
                output.push(std::move(team_clone));
            }
            else {
                break;
            }

            while (output.size() > max_keep)
                output.pop();
        }

        // Check stop conditions.
        if (best_score <= OPTIONS.ga_stop_score) {
            // Check needed for fprintf().
            if (JUtil.check_log_lvl(LOGLVL_INFO)) {
                fprintf(
                    stdout,
                    "---------------------------------------------\n");
                JUtil.info(
                    "Solver reached stopping score %.2f!\n"
                    "---------------------------------------------\n",
                    OPTIONS.ga_stop_score);
            }
            score_satisfied_ = true;
        }
        else {
            if (OPTIONS.ga_restart_trigger == 0 or
                    stagnant_count < OPTIONS.ga_restart_trigger) {
                JUtil.info("Current stagnancy: %zu (max=%zu)\n\n",
                           stagnant_count,
                           OPTIONS.ga_restart_trigger);
            }
            else {
                if (JUtil.check_log_lvl(LOGLVL_WARNING)) {
                    fprintf(stdout, "\n");
                    JUtil.warn(
                        "Stagnancy reached restart trigger (%zu)\n",
                        OPTIONS.ga_restart_trigger);
                }
                should_restart_ga_ = true;
            }
        }
    }

    /* printers */
    void print_start_msg(WorkArea const& wa) const
    {
        JUtil.info("Solving for work are \"%s\"\n", wa.name.c_str());
        JUtil.info("Length guess=%zu; Spec has %d points\n",
                   wa.target_size, wa.path_len);
        JUtil.info("Using deviation allowance: %d nodes\n", OPTIONS.len_dev);
        JUtil.info("Max Iterations: %zu\n", OPTIONS.ga_max_iters);
        JUtil.info("Surviors: %u\n", CUTOFFS.survivors);

        JUtil.info("There are %d devices. Host ID=%d; currently using ID=%d\n",
                   omp_get_num_devices(), omp_get_initial_device(), OPTIONS.device);
        omp_set_default_device(OPTIONS.device);

        #pragma omp parallel
        {
            #pragma omp single
            {
                JUtil.info("GA starting with %d threads\n\n", omp_get_num_threads());
            }
        }
    }

    void print_end_msg() const {
        if (restart_id != 0 and restart_id == OPTIONS.ga_max_restarts) {
            JUtil.warn("Reached max restarts (%zu)\n", OPTIONS.ga_max_restarts);
        }

        JUtil.info("Total: %zu iterations\n", itr_id);

        double const time_elapsed_in_us = JUtil.get_timestamp_us() - start_time_in_us_;
        size_t const minutes = std::floor(time_elapsed_in_us / 1e6 / 60.0f);
        size_t const seconds = std::floor(fmod(time_elapsed_in_us / 1e6, 60.0f));
        size_t const milliseconds = std::floor(fmod(time_elapsed_in_us / 1e3, 1000.0f));
        JUtil.info("EvolutionSolver finished in %zum %zus %zums\n",
                   minutes, seconds, milliseconds);
    }


#define PRINT_POP_FMT \
        "%s #%zu [cksm:%x] [len:%zu] [score:%.2f]\n"
    void print_pop(std::string const& title,
                   Population const& pop) const
    {
        size_t const max_n =
            std::min(debug_pop_print_n_, pop.front_buffer()->size());

        if (max_n) {
            std::ostringstream oss;
            oss << title << "\n";

            for (size_t i = 0; i < max_n; ++i) {
                auto& c = pop.front_buffer()->at(i);
                oss << string_format(PRINT_POP_FMT,
                                     "  front",
                                     i,
                                     c->checksum(),
                                     c->size(),
                                     c->score());
            }

            for (size_t i = 0; i < max_n; ++i) {
                auto& c = pop.back_buffer()->at(i);
                oss << string_format(PRINT_POP_FMT,
                                     "  back ",
                                     i,
                                     c->checksum(),
                                     c->size(),
                                     c->score());
            }

            JUtil.debug(oss.str().c_str());
        }
    }
#undef PRINT_POP_FMT

    void run(WorkArea const& work_area, TeamSPMaxHeap& output) {
        reset();
        start_time_in_us_ = JUtil.get_timestamp_us();

        // Activate ProtoTerm profile if there is one.
        InputManager::mutable_xdb().activate_ptterm_profile(work_area.ptterm_profile);

        auto seed = OPTIONS.seed;  // Reentrant seed.

        while (OPTIONS.ga_max_restarts == 0 or
                restart_id < OPTIONS.ga_max_restarts) {
            should_restart_ga_ = false;

            // Initialize population and solution list.
            Population population = Population(&work_area, seed);

            print_start_msg(work_area);

            tot_gen_time = 0.0f;
            stagnant_count = 0;

            if (OPTIONS.dry_run) break;

            gen_id = 0;
            while (OPTIONS.ga_max_iters == 0 or itr_id < OPTIONS.ga_max_iters) {
                double const gen_start_time = JUtil.get_timestamp_us();

                population.evolve();
                print_pop("After evolve", population);

                population.rank();
                print_pop("Post rank", population);

                population.select();
                print_pop("Post select", population);

                summarize_generation(population,
                                        gen_start_time,
                                        output);

                if (should_restart_ga_ or score_satisfied_) break;

                population.swap_buffer();

                gen_id++;
                itr_id++;
            }  // generation

            if (score_satisfied_) break;
            restart_id++;
        }  // restart

        print_end_msg();
    }
};

/* public */
/* ctors */
EvolutionSolver::EvolutionSolver(size_t const debug_pop_print_n) :
    pimpl_(std::make_unique<PImpl>(debug_pop_print_n)) {}

/* dtors */
EvolutionSolver::~EvolutionSolver() {}

/* modifiers */
void EvolutionSolver::run(WorkArea const& work_area, TeamSPMaxHeap& output) {
    return pimpl_->run(work_area, output);
}

} // namespace elfin
