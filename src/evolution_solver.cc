#include "evolution_solver.h"

#include <cmath>
#include <sstream>
#include <algorithm>
#include <stdlib.h>

#include <jutil/jutil.h>
#include "input_manager.h"
#include "parallel_utils.h"

namespace elfin {

auto get_score_msg_format = []() {
    size_t gen_digits = std::ceil(std::log(OPTIONS.ga_iters) / std::log(10));
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
    bool run_called = false;
    SolutionMap best_sols_;
    double start_time_in_us_ = 0;
    size_t const debug_pop_print_n_;
    char const* const print_pop_fmt_ =
        "%s #%zu [cksm:%p] [len:%zu] [score:%.2f]\n";

    /* ctors */
    PImpl(size_t const debug_pop_print_n) :
        debug_pop_print_n_(debug_pop_print_n) {}

    /* modifiers */
    bool collect_gen_data(
        Population const& pop,
        size_t const gen_id,
        double const gen_start_time,
        double& tot_gen_time,
        size_t& stagnant_count,
        float& lastgen_best_score,
        NodeTeamSPList& best_sols) {
        bool should_stop_ga = false;

        // Stat collection
        auto const& best_team = pop.front_buffer()->front();
        auto const& worst_team = pop.front_buffer()->back();

        float const gen_best_score = best_team->score();
        double const gen_time =
            (get_timestamp_us() - gen_start_time) / 1e3;

        tot_gen_time += gen_time;

        // Print score stats.
        info(get_score_msg_format().c_str(),
            gen_id,
            gen_best_score,
            gen_best_score / best_team->size(),
            worst_team->score(),
            gen_time);

        // Compute stagnancy & check inverted scores.
        if (float_approximates(gen_best_score, lastgen_best_score)) {
            stagnant_count++;
        }
        else if (gen_best_score > lastgen_best_score) {
            err("Best score is worse than last gen!\n");
            print_pop("Error debug", pop);
            NICE_PANIC("Score ranking bug?");
        }
        else {
            stagnant_count = 0;
        }

        lastgen_best_score = gen_best_score;

        // Print timing stats.
        size_t const n_gens = gen_id + 1;
        info(timing_msg_format.c_str(),
            (double) GA_TIMES.evolve_time / n_gens,
            (double) GA_TIMES.score_time / n_gens,
            (double) GA_TIMES.rank_time / n_gens,
            (double) GA_TIMES.select_time / n_gens,
            (double) tot_gen_time / n_gens);

        // Update best solutions.
        best_sols = NodeTeamSPList();
        for (size_t j = 0; j < OPTIONS.keep_n; j++) {
            best_sols.emplace_back(pop.front_buffer()->at(j)->clone());
            has_result_ |= true;
        }

        // Check stop conditions.
        if (gen_best_score < OPTIONS.ga_stop_score) {
            gated_raw(LOG_INFO, "\n");
            info("Score stopping threshold %.2f reached\n",
                OPTIONS.ga_stop_score);
            should_stop_ga = true;
        }
        else {
            if (OPTIONS.ga_stop_stagnancy != -1 and
                    stagnant_count >= OPTIONS.ga_stop_stagnancy) {
                gated_raw(LOG_INFO, "\n");
                warn("Solver stopped because max stagnancy is reached (%d)\n", OPTIONS.ga_stop_stagnancy);
                should_stop_ga = true;
            }
            else {
                info("Current stagnancy: %d, max: %d\n\n", stagnant_count, OPTIONS.ga_stop_stagnancy);
            }
        }

        return should_stop_ga;
    }

    /* printers */
    void print_start_info(WorkArea const& wa) const {
        info("Length guess: < %zu; Spec has %d points\n",
            wa.target_size(), wa.points().size());
        info("Using deviation allowance: %d nodes\n", OPTIONS.len_dev);
        info("Max Iterations: %zu\n", OPTIONS.ga_iters);
        info("Surviors: %u\n", CUTOFFS.survivors);

        info("There are %d devices. Host ID=%d; currently using ID=%d\n",
            omp_get_num_devices(), omp_get_initial_device(), OPTIONS.device);
        omp_set_default_device(OPTIONS.device);

        #pragma omp parallel
        {
            #pragma omp single
            {
                info("GA starting with %d threads\n\n", omp_get_num_threads());
            }
        }
    }

    void print_end_info() const {
        double const time_elapsed_in_us = get_timestamp_us() - start_time_in_us_;
        size_t const minutes = std::floor(time_elapsed_in_us / 1e6 / 60.0f);
        size_t const seconds = std::floor(fmod(time_elapsed_in_us / 1e6, 60.0f));
        size_t const milliseconds = std::floor(fmod(time_elapsed_in_us / 1e3, 1000.0f));
        info("EvolutionSolver finished in %zum %zus %zums\n",
            minutes, seconds, milliseconds);
    }

    void print_pop(
        std::string const& title,
        Population const& pop) const {
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

            dbg(oss.str().c_str());
        }
    }

    void run() {
        NICE_PANIC(run_called, "GA run() called more than once.\n");
        run_called = true;

        start_time_in_us_ = get_timestamp_us();
        for (auto& itr : SPEC.work_areas()) {
            std::string const wa_name = itr.first;
            auto& wa = itr.second;
            if (wa->type() != WorkType::FREE and
                    wa->type() != WorkType::ONE_HINGE) {
                std::ostringstream ss;
                ss << "Skipping work_area: ";
                ss << WorkTypeToCStr(wa->type()) << std::endl;
                warn(ss.str().c_str());
                continue;
            }

            Population population = Population(wa.get());

            print_start_info(*wa);

            double tot_gen_time = 0.0f;
            size_t stagnant_count = 0;
            float lastgen_best_score = INFINITY;

            if (!OPTIONS.dry_run) {
                for (size_t gen_id = 0; gen_id < OPTIONS.ga_iters; gen_id++) {
                    double const gen_start_time = get_timestamp_us();

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
                }
            }
        }

        print_end_info();
    }
};

/* public */
/* ctors */
EvolutionSolver::EvolutionSolver(size_t const debug_pop_print_n) :
    p_impl_(std::make_unique<PImpl>(debug_pop_print_n)) {}

/* dtors */
EvolutionSolver::~EvolutionSolver() {}

/* accessors */
bool EvolutionSolver::has_result() const {
    return p_impl_->has_result_;
}

SolutionMap const& EvolutionSolver::best_sols() const {
    return p_impl_->best_sols_;
}

/* modifiers */
void EvolutionSolver::run() {
    p_impl_->run();
}

} // namespace elfin
