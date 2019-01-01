#include "evolution_solver.h"

#include <cmath>
#include <sstream>
#include <algorithm>
#include <stdlib.h>

#include "jutil.h"
#include "input_manager.h"
#include "parallel_utils.h"

namespace elfin {

auto get_score_msg_format = []() {
    size_t const gen_digits =
        gen_digits == 0 ? 5 :
        std::ceil(std::log(OPTIONS.ga_iters) / std::log(10));
    return string_format(
               ("Generation #%%%zuzu: "
                "best=%%.2f (%%.2f/module), "
                "worst=%%.2f, time taken=%%.0fms\n"),
               gen_digits);
};
std::string timing_msg_format =
    ("Mean Times: Evolve=%.0f, "
     "Score=%.0f, Rank=%.0f, "
     "Select=%.0f, Gen=%.0f\n");

/* private */
struct EvolutionSolver::PImpl {
    /* data */
    bool has_result_ = false;
    bool run_alread_called = false;
    SolutionMap best_sols_;
    double start_time_in_us_ = 0;
    size_t const debug_pop_print_n_;
    char const* const print_pop_fmt_ =
        "%s #%zu [cksm:%p] [len:%zu] [score:%.2f]\n";

    /* ctors */
    PImpl(size_t const debug_pop_print_n) :
        debug_pop_print_n_(debug_pop_print_n) {}

    /* modifiers */
    bool collect_gen_data(Population const& pop,
                          size_t const gen_id,
                          double const gen_start_time,
                          double& tot_gen_time,
                          size_t& stagnant_count,
                          float& lastgen_best_score,
                          NodeTeamSPList& best_sols)
    {
        bool should_stop_ga = false;

        // Stat collection
        auto const& best_team = pop.front_buffer()->front();
        auto const& worst_team = pop.front_buffer()->back();

        float const gen_best_score = best_team->score();
        double const gen_time =
            (JUtil.get_timestamp_us() - gen_start_time) / 1e3;

        tot_gen_time += gen_time;

        // Print score stats.
        JUtil.info(get_score_msg_format().c_str(),
                   gen_id,
                   gen_best_score,
                   gen_best_score / best_team->size(),
                   worst_team->score(),
                   gen_time);

        // Compute stagnancy & check inverted scores.
        if (JUtil.float_approximates(gen_best_score, lastgen_best_score, 1e-6)) {
            stagnant_count++;
        }
        else {
            if (gen_best_score > lastgen_best_score) {
                print_pop("Error debug", pop);
                TRACE_NOMSG("Best score is worse than last gen.\n");
            }
            stagnant_count = 0;
        }

        lastgen_best_score = gen_best_score;

        // Print timing stats.
        size_t const n_gens = gen_id + 1;
        JUtil.info(timing_msg_format.c_str(),
                   (double) GA_TIMES.evolve_time / n_gens,
                   (double) GA_TIMES.score_time / n_gens,
                   (double) GA_TIMES.rank_time / n_gens,
                   (double) GA_TIMES.select_time / n_gens,
                   (double) tot_gen_time / n_gens);

        // Update best solutions.
        best_sols.clear();
        size_t const max_keep = std::min(OPTIONS.keep_n, OPTIONS.ga_pop_size);
        for (size_t j = 0; j < max_keep; j++) {
            best_sols.emplace_back(pop.front_buffer()->at(j)->clone());
            has_result_ |= true;
        }

        // Check stop conditions.
        if (gen_best_score < OPTIONS.ga_stop_score) {
            if (JUtil.check_log_lvl(LOGLVL_INFO)) {
                fprintf(stdout, "---------------------------------------------\n");
            }
            JUtil.info("Solver reached stopping score %.2f!\n"
                       "---------------------------------------------\n",
                       OPTIONS.ga_stop_score);
            should_stop_ga = true;
        }
        else {
            if (OPTIONS.ga_stop_stagnancy == 0 or
                    stagnant_count < OPTIONS.ga_stop_stagnancy) {
                JUtil.info("Current stagnancy: %zu, max: %ld\n\n",
                           stagnant_count,
                           OPTIONS.ga_stop_stagnancy);
            }
            else {
                fprintf(stdout, "\n");
                JUtil.warn(
                    "Solver reached stagnancy limit (%ld)\n",
                    OPTIONS.ga_stop_stagnancy);
                should_stop_ga = true;
            }
        }

        return should_stop_ga;
    }

    /* printers */
    void print_start_msg(WorkArea const& wa) const
    {
        JUtil.info("Length guess=%zu; Spec has %d points\n",
                   wa.target_size, wa.points.size());
        JUtil.info("Using deviation allowance: %d nodes\n", OPTIONS.len_dev);
        JUtil.info("Max Iterations: %zu\n", OPTIONS.ga_iters);
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
                oss << string_format(print_pop_fmt_,
                                     "  front", i, c->checksum(), c->size(), c->score());
            }

            for (size_t i = 0; i < max_n; ++i) {
                auto& c = pop.back_buffer()->at(i);
                oss << string_format(print_pop_fmt_,
                                     "  back ", i, c->checksum(), c->size(), c->score());
            }

            JUtil.debug(oss.str().c_str());
        }
    }

    void run() {
        TRACE_NOMSG(run_alread_called);
        run_alread_called = true;

        start_time_in_us_ = JUtil.get_timestamp_us();
        for (auto& [wa_name, wa] : SPEC.work_areas()) {
            // Initialize population and solution list.
            Population population = Population(wa.get());
            best_sols_[wa_name] = NodeTeamSPList();

            print_start_msg(*wa);

            double tot_gen_time = 0.0f;
            size_t stagnant_count = 0;
            float lastgen_best_score = INFINITY;

            if (!OPTIONS.dry_run) {
                size_t gen_id = 0;
                while (OPTIONS.ga_iters == 0 or gen_id < OPTIONS.ga_iters) {
                    double const gen_start_time = JUtil.get_timestamp_us();

                    population.evolve();
                    print_pop("After evolve", population);

                    population.rank();
                    print_pop("Post rank", population);

                    population.select();
                    print_pop("Post select", population);

                    bool const should_break = collect_gen_data(
                                                  population,
                                                  gen_id,
                                                  gen_start_time,
                                                  tot_gen_time,
                                                  stagnant_count,
                                                  lastgen_best_score,
                                                  best_sols_[wa_name]);

                    if (should_break) break;

                    population.swap_buffer();

                    gen_id++;
                }
            }
        }

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

SolutionMap const& EvolutionSolver::best_sols() const {
    return pimpl_->best_sols_;
}

/* modifiers */
void EvolutionSolver::run() {
    pimpl_->run();
}

} // namespace elfin
