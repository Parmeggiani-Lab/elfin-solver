#include "evolution_solver.h"

#include <cmath>
#include <sstream>
#include <algorithm>
#include <stdlib.h>
#include <unordered_map>
#include <limits>

#include "jutil.h"
#include "input_manager.h"
#include "parallel_utils.h"

namespace elfin {

auto get_score_msg_format = []() {
    size_t gen_digits = std::ceil(std::log(OPTIONS.ga_iters) / std::log(10));
    return string_format(
               ("Generation #%%%lulu: "
                "best=%%.2f (%%.2f/module), "
                "worst=%%.2f, time taken=%%.0fms\n"),
               gen_digits);
};
std::string timing_msg_format =
    ("Mean Times: Evolve=%.0f, "
     "Score=%.0f, Rank=%.0f, "
     "Select=%.0f, Gen=%.0f\n");

/* Private Methods */

#ifdef _VTUNE
#include <ittnotify.h>
#endif

void
EvolutionSolver::collect_gen_data(
    const Population& pop,
    const size_t gen_id,
    const double gen_start_time,
    double& tot_gen_time,
    size_t& stagnant_count,
    float& lastgen_best_score,
    CandidateSharedPtrs& best_sols,
    bool& should_break) {
    // Stat collection
    const Candidate* best_candidate =
        pop.front_buffer()->front();
    const Candidate* worst_candidate =
        pop.front_buffer()->back();

    const float gen_best_score =
        best_candidate->score();
    const size_t gen_best_len =
        best_candidate->size();
    const float gen_worst_score =
        worst_candidate->score();
    const double gen_time =
        ((get_timestamp_us() - gen_start_time) / 1e3);

    tot_gen_time += gen_time;

    // Print score stats
    msg(get_score_msg_format().c_str(),
        gen_id,
        gen_best_score,
        gen_best_score / gen_best_len,
        gen_worst_score,
        gen_time);

    // Compute stagnancy& check inverted scores
    if (float_approximates(gen_best_score, lastgen_best_score)) {
        stagnant_count++;
    }
    else if (gen_best_score > lastgen_best_score) {
        err("Best score is worse than last gen!\n");
        debug_print_pop(pop, 16);
        NICE_PANIC("Score ranking bug?");
    }
    else {
        stagnant_count = 0;
    }

    lastgen_best_score = gen_best_score;

    // Print timing stats
    const size_t n_gens = gen_id + 1;
    msg(timing_msg_format.c_str(),
        (double) GA_TIMES.evolve_time / n_gens,
        (double) GA_TIMES.score_time / n_gens,
        (double) GA_TIMES.rank_time / n_gens,
        (double) GA_TIMES.select_time / n_gens,
        (double) tot_gen_time / n_gens);

    // update best sols
    for (size_t j = 0; j < OPTIONS.keep_n; j++) {
        Candidate* best_cand_clone =
            pop.front_buffer()->at(j)->clone();
        best_sols[j] =
            std::shared_ptr<Candidate>(best_cand_clone);
        has_result_ |= true;
    }

    // Check stop conditions
    if (gen_best_score < OPTIONS.ga_stop_score) {
        msg("Score stop threshold %.2f reached\n",
            OPTIONS.ga_stop_score);
        should_break = true;
        return;
    }

    if (OPTIONS.ga_stop_stagnancy != -1 and
            stagnant_count >= OPTIONS.ga_stop_stagnancy) {
        wrn("Solver stopped because max stagnancy is reached (%d)\n", OPTIONS.ga_stop_stagnancy);
        should_break = true;
        return;
    }

    msg("Current stagnancy: %d, max: %d\n", stagnant_count, OPTIONS.ga_stop_stagnancy);
}

void
EvolutionSolver::print_start_msg(const V3fList& shape) const {
    for (auto& p : shape)
        dbg("Work Area Point: %s\n", p.to_string().c_str());

    msg("Length guess: < %lu; Spec has %d points\n",
        Candidate::MAX_LEN,
        shape.size());
    msg("Using deviation allowance: %d nodes\n", OPTIONS.len_dev_alw);

    // Format big numbers nicely
    std::string pop_size_str;
    const size_t pop_size = OPTIONS.ga_pop_size;
    if (pop_size >= 1e6) {
        pop_size_str = string_format("%.1fM", (float) (pop_size / 1e6));
    }
    else if (pop_size >= 1e3) {
        pop_size_str = string_format("%.1fK", (float) (pop_size / 1e3));
    }
    else {
        pop_size_str = string_format("%lu", pop_size);
    }

    std::string max_iters_str;
    if (OPTIONS.ga_iters >= 1e3) {
        max_iters_str = string_format("%.1fK", (float) (OPTIONS.ga_iters / 1e3));
    }
    else {
        max_iters_str = string_format("%lu", OPTIONS.ga_iters);
    }


    msg(("Elfin will run with:\n"
         "  Population size:  %s\n"
         "  Max Iterations:   %s\n"
         "  Surviors:         %u\n"),
        pop_size_str.c_str(),
        max_iters_str.c_str(),
        CUTOFFS.survivors);

    const int n_omp_devices = omp_get_num_devices();
    const int host_device_id = omp_get_initial_device();
    msg("There are %d devices. Host ID=%d; currently using #%d\n", n_omp_devices, host_device_id, OPTIONS.device);
    omp_set_default_device(OPTIONS.device);

    #pragma omp parallel
    {
        if (omp_get_thread_num() == 0)
            msg("Running with %d threads\n", omp_get_max_threads());
    }
}

void
EvolutionSolver::print_end_msg() const {
    msg("EvolutionSolver finished: ");
    this->print_timing();
}

void
EvolutionSolver::print_timing() const {
    const double time_elapsed_in_us = get_timestamp_us() - start_time_in_us_;
    const uint64_t minutes = std::floor(time_elapsed_in_us / 1e6 / 60.0f);
    const uint64_t seconds = std::floor(fmod(time_elapsed_in_us / 1e6, 60.0f));
    const uint64_t milliseconds = std::floor(fmod(time_elapsed_in_us / 1e3, 1000.0f));
    raw("%um %us %ums\n",
        minutes, seconds, milliseconds);
}

void
EvolutionSolver::debug_print_pop(
    const Population& pop,
    const size_t cutoff) const {
    const size_t i_max =
        std::min(cutoff, pop.front_buffer()->size());

    for (size_t i = 0; i < i_max; ++i)
    {
        auto& c = pop.front_buffer()->at(i);
        wrn("curr  [#%lu:%p] [cksm:%p] [score:%.2f] [len:%lu]\n",
            i, c, c->checksum(), c->score(), c->size());
    }

    for (size_t i = 0; i < i_max; ++i)
    {
        auto& c = pop.back_buffer()->at(i);
        wrn("buff  [#%lu:%p] [cksm:%p] [score:%.2f] [len:%lu]\n",
            i, c, c->checksum(), c->score(), c->size());
    }
}

/* Public Methods */

void
EvolutionSolver::run() {
    static bool run_entered = false;
    
    if (run_entered) {
        die("%s called more than once.\n", __PRETTY_FUNCTION__);
    }

    run_entered = true;

    start_time_in_us_ = get_timestamp_us();
    for (auto& itr : SPEC.work_area_map()) {

        // if this was a complex work area, we need to break it down to
        // multiple simple ones by first choosing hubs and their orientations.
        /*
        TODO: Break complex work area
        */
        const std::string wa_name = itr.first;
        const WorkArea& wa = itr.second;
        if (wa.type() != WorkType::FREE) {
            std::ostringstream ss;
            ss << "Skipping work_area: ";
            ss << WorkTypeToCStr(wa.type()) << std::endl;
            wrn(ss.str().c_str());
            continue;
        }

        Population population = Population(&wa);

        const V3fList shape = itr.second.to_points();
        this->print_start_msg(shape);

        best_sols_[wa_name] = CandidateSharedPtrs();
        best_sols_[wa_name].resize(OPTIONS.keep_n);

        double tot_gen_time = 0.0f;
        size_t stagnant_count = 0;
        float lastgen_best_score = INFINITY;

        if (!OPTIONS.dry_run) {
            MAP_DATA() {
                for (size_t gen_id = 0; gen_id < OPTIONS.ga_iters; gen_id++) {
                    const double gen_start_time = get_timestamp_us();

                    wrn("Before evolve\n");
                    debug_print_pop(population);

                    population.evolve();
                    wrn("After evolve\n");
                    debug_print_pop(population);

                    // population.score();

                    population.rank();
                    wrn("Post rank\n");
                    debug_print_pop(population);

                    population.select();
                    wrn("Post select\n");
                    debug_print_pop(population);

                    bool should_break = false;
                    collect_gen_data(
                        population,
                        gen_id,
                        gen_start_time,
                        tot_gen_time,
                        stagnant_count,
                        lastgen_best_score,
                        best_sols_[wa_name],
                        should_break);
                    if (should_break) break;

                    population.swap_buffer();
                }
            }
        }
    }

    this->print_end_msg();
}

} // namespace elfin
