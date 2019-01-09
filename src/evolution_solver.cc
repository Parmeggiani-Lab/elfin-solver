#include "evolution_solver.h"

#include <cmath>
#include <sstream>
#include <algorithm>
#include <stdlib.h>

#include "jutil.h"
#include "input_manager.h"
#include "parallel_utils.h"

namespace elfin {

char const* const get_score_msg_format =
    "GA Restart #%zu Generation #%zu Total Iterations #%zu\n"
    "  -  [best %.2f (cksm:%x, %.2f/module), worst %.2f, %.0fms]\n";

char const* const timing_msg_format =
    "Avg Times: "
    "[Evolve=%.0f, Score=%.0f, Rank=%.0f, "
    "Select=%.0f, Gen=%.0f]\n";

char const* const print_pop_fmt =
    "%s #%zu [cksm:%x] [len:%zu] [score:%.2f]\n";

/* private */
struct EvolutionSolver::PImpl {
    /* data */
    bool score_satisfied_ = false;
    bool should_restart_ga = false;
    bool has_result_ = false;
    bool run_already_called = false;
    SolutionMap best_sols_;
    double start_time_in_us_ = 0;
    size_t const debug_pop_print_n_;
    Crc32 last_best_checksum_ = 0x0000;

    /* ctors */
    PImpl(size_t const debug_pop_print_n) :
        debug_pop_print_n_(debug_pop_print_n) {}

    /* modifiers */
    void summarize_generation(Population const& pop,
                              size_t const restart_id,
                              size_t const gen_id,
                              size_t const itr_id,
                              double const gen_start_time,
                              double& tot_gen_time,
                              size_t& stagnant_count,
                              TeamSPMinHeap& best_sols)
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
        JUtil.info(get_score_msg_format,
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
        JUtil.info(timing_msg_format,
                   (double) GA_TIMES.evolve_time / n_gens,
                   (double) GA_TIMES.score_time / n_gens,
                   (double) GA_TIMES.rank_time / n_gens,
                   (double) GA_TIMES.select_time / n_gens,
                   (double) tot_gen_time / n_gens);

        // Update best solutions.
        size_t const max_keep = std::min(OPTIONS.keep_n, OPTIONS.ga_pop_size);
        for (size_t j = 0; j < max_keep; j++) {
            auto team = pop.front_buffer()->at(j)->clone();
            if (best_sols.size() >= max_keep) {
                if (team->score() >= best_sols.top()->score()) {
                    // Due to increasing index, teams down the buffer are just
                    // gonna get worse.
                    break;
                }

                best_sols.pop();
            }

            best_sols.push(std::move(team));
            has_result_ |= true;
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
                fprintf(stdout, "\n");
                JUtil.warn(
                    "Stagnancy reached restart trigger (%zu)\n",
                    OPTIONS.ga_restart_trigger);
                should_restart_ga = true;
            }
        }
    }

    /* printers */
    void print_start_msg(WorkArea const& wa) const
    {
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
        double const time_elapsed_in_us = JUtil.get_timestamp_us() - start_time_in_us_;
        size_t const minutes = std::floor(time_elapsed_in_us / 1e6 / 60.0f);
        size_t const seconds = std::floor(fmod(time_elapsed_in_us / 1e6, 60.0f));
        size_t const milliseconds = std::floor(fmod(time_elapsed_in_us / 1e3, 1000.0f));
        JUtil.info("EvolutionSolver finished in %zum %zus %zums\n",
                   minutes, seconds, milliseconds);
    }

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
                oss << string_format(print_pop_fmt,
                                     "  front",
                                     i,
                                     c->checksum(),
                                     c->size(),
                                     c->score());
            }

            for (size_t i = 0; i < max_n; ++i) {
                auto& c = pop.back_buffer()->at(i);
                oss << string_format(print_pop_fmt,
                                     "  back ",
                                     i,
                                     c->checksum(),
                                     c->size(),
                                     c->score());
            }

            JUtil.debug(oss.str().c_str());
        }
    }

    void run() {
        TRACE_NOMSG(run_already_called);
        run_already_called = true;

        start_time_in_us_ = JUtil.get_timestamp_us();
        for (auto& [wa_name, wa] : SPEC.work_areas()) {
            best_sols_[wa_name] = TeamSPMinHeap();

            // Activate ProtoTerm profile if there is one.
            InputManager::mutable_xdb().activate_ptterm_profile(wa->ptterm_profile);
            for (auto& mod : XDB.all_mods()) {
                mod->finalize();
            }

            size_t itr_id = 0;
            size_t restart_id = 0;
            while (OPTIONS.ga_max_restarts == 0 or
                    restart_id < OPTIONS.ga_max_restarts) {
                should_restart_ga = false;

                // Initialize population and solution list.
                Population population = Population(wa.get());

                print_start_msg(*wa);

                double tot_gen_time = 0.0f;
                size_t stagnant_count = 0;

                if (!OPTIONS.dry_run) {
                    size_t gen_id = 0;
                    while (OPTIONS.ga_max_iters == 0 or itr_id < OPTIONS.ga_max_iters) {
                        double const gen_start_time = JUtil.get_timestamp_us();

                        population.evolve();
                        print_pop("After evolve", population);

                        population.rank();
                        print_pop("Post rank", population);

                        population.select();
                        print_pop("Post select", population);

                        summarize_generation(population,
                                             restart_id,
                                             gen_id,
                                             itr_id,
                                             gen_start_time,
                                             tot_gen_time,
                                             stagnant_count,
                                             best_sols_[wa_name]);

                        if (should_restart_ga or score_satisfied_) break;

                        population.swap_buffer();

                        gen_id++;
                        itr_id++;
                    }  // generation
                }  // dry run

                if (score_satisfied_) break;
                restart_id++;
            }  // restart

            if (restart_id != 0 and restart_id == OPTIONS.ga_max_restarts) {
                JUtil.warn("Reached max restarts (%zu)\n", OPTIONS.ga_max_restarts);
            }

            JUtil.info("Total: %zu iterations\n", itr_id);
        }  // work area

        print_end_msg();
    }
};

/* public */
/* ctors */
EvolutionSolver::EvolutionSolver(size_t const debug_pop_print_n) :
    pimpl_(std::make_unique<PImpl>(debug_pop_print_n)) {}

/* dtors */
EvolutionSolver::~EvolutionSolver() {}

/* accessors */
bool EvolutionSolver::has_result() const {
    return pimpl_->has_result_;
}

TeamPtrMaxHeap EvolutionSolver::best_sols(std::string const& wa_name) const {
    TeamPtrMaxHeap res;
    TeamSPMinHeap tmp;
    TeamSPMinHeap& heap = pimpl_->best_sols_.at(wa_name);

    while (not heap.empty()) {
        auto node_sp = heap.top_and_pop();
        res.push(node_sp.get());
        tmp.push(std::move(node_sp));
    }

    // Transfer back
    while (not tmp.empty()) {
        heap.push(tmp.top_and_pop());
    }

    return res;
}

/* modifiers */
void EvolutionSolver::run() {
    pimpl_->run();
}

} // namespace elfin
