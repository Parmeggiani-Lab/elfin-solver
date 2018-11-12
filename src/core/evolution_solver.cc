#include "evolution_solver.h"

#include <cmath>
#include <sstream>
#include <algorithm>
#include <stdlib.h>
#include <unordered_map>
#include <limits>

#include "shorthands.h"
#include "options.h"
#include "spec.h"
#include "radii.h"
#include "jutil.h"
#include "parallel_utils.h"

#define DEBUG_PRINT_POP 4

namespace elfin {

/* Private Methods */

#ifdef _VTUNE
#include <ittnotify.h>
#endif

void
EvolutionSolver::init_pop_buffs(const WorkArea & wa) {
    curr_pop_ = new Population(wa);
    buff_pop_ = new Population(*curr_pop_);
}

void
EvolutionSolver::swap_pop_buffs() {
    const Population * tmp = buff_pop_;
    buff_pop_ = curr_pop_;
    curr_pop_ = const_cast<Population *>(tmp);
}

void
EvolutionSolver::print_start_msg(const Points3f & shape) const {
    for (auto & p : shape)
        dbg("Work Area Point: %s\n", p.to_string().c_str());

    msg("Length guess: <%lu, spec has %d points\n",
        CANDIDATE_LENGTHS.max,
        shape.size());
    msg("Using deviation allowance: %d nodes\n", OPTIONS.len_dev_alw);

    // Want auto significant figure detection with streams
    std::ostringstream popsize_ss;
    const ulong pop_size = OPTIONS.ga_pop_size;
    if (pop_size > 1000)
        popsize_ss << (float) (pop_size / 1000.0f) << "k";
    else
        popsize_ss << pop_size;

    std::ostringstream nitr_ss;
    if (OPTIONS.ga_iters > 1000)
        nitr_ss << (float) (OPTIONS.ga_iters / 1000.0f) << "k";
    else
        nitr_ss << OPTIONS.ga_iters;


    msg("EvolutionSolver starting with following settings:\n"
        "Population size:            %s\n"
        "Iterations:                 %s\n"
        "Survive cutoff:             %u\n"
        "Cross cutoff:               %u\n"
        "Point Mutate cutoff:        %u\n"
        "Limb Mutate cutoff:         %u\n"
        "New species:                %u\n",
        popsize_ss.str().c_str(),
        nitr_ss.str().c_str(),
        POPULATION_COUNTERS.survivors,
        MUTATION_CUTOFFS.cross,
        MUTATION_CUTOFFS.point,
        MUTATION_CUTOFFS.limb,
        pop_size - MUTATION_CUTOFFS.limb);

    const int n_omp_devices = omp_get_num_devices();
    const int host_device_id = omp_get_initial_device();
    msg("There are %d devices. Host is #%d; currently using #%d\n", n_omp_devices, host_device_id, OPTIONS.device);
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
EvolutionSolver::debug_print_pop(size_t cutoff) const {
    size_t count = 0;
    for (auto & c : curr_pop_->candidates()) {
        wrn("c [#%lu] [cksm:%x] [score:%.2f] [len:%lu]\n",
            count, c->checksum(), c->get_score(), c->nodes().size());
        count++;
        if (count >= cutoff)
            break;
    }

    count = 0;
    for (auto & c : buff_pop_->candidates()) {
        wrn("bc [#%lu] [cksm:%x] [score:%.2f] [len:%lu]\n",
            count, c->checksum(), c->get_score(), c->nodes().size());
        count++;
        if (count >= cutoff)
            break;
    }
}

/* Public Methods */

void
EvolutionSolver::run() {
    start_time_in_us_ = get_timestamp_us();
    for (auto & itr : SPEC.work_areas()) {

        // if this was a complex work area, we need to break it down to
        // multiple simple ones by first choosing hubs and their orientations.
        /*
        TODO: Break complex work area
        */
        const std::string wa_name = itr.first;
        const WorkArea & wa = itr.second;
        if (wa.type() != FREE) {
            std::stringstream ss;
            ss << "Skipping work_area: ";
            ss << WorkTypeNames[wa.type()] << std::endl;
            wrn(ss.str().c_str());
            continue;
        }

        Population::setup(wa);
        init_pop_buffs(wa);

        const Points3f shape = itr.second.to_points3f();
        this->print_start_msg(shape);

        best_sols_[wa_name] = CandidateSharedPtrs();
        best_sols_[wa_name].resize(OPTIONS.n_best_sols);

        float lastgen_best_score = std::numeric_limits<float>::infinity();
        uint stagnant_count = 0;

        const int genDispDigits = std::ceil(std::log(OPTIONS.ga_iters) / std::log(10));
        char * gen_msg_fmt;
        int asprintf_ret = -1;
        asprintf_ret = asprintf(&gen_msg_fmt,
                                "Generation #%%%dd: best=%%.2f (%%.2f/module), worst=%%.2f, time taken=%%.0fms\n", genDispDigits);
        char * avg_time_msg_fmt;
        asprintf_ret = asprintf(&avg_time_msg_fmt,
                                "Avg Times: Evolve=%%.0f,Score=%%.0f,Rank=%%.0f,Select=%%.0f,Gen=%%.0f\n");

        double tot_gen_time = 0.0f;
        if (!OPTIONS.dry_run) {
            MAP_DATA() {
                for (int i = 0; i < OPTIONS.ga_iters; i++) {
                    const double genStartTime = get_timestamp_us();

#if DEBUG_PRINT_POP > 0
                    wrn("Before evolve\n");
                    debug_print_pop(DEBUG_PRINT_POP);
#endif // DEBUG_PRINT_POP

                    curr_pop_->evolve(buff_pop_);
                    curr_pop_->score();
                    curr_pop_->rank();

#if DEBUG_PRINT_POP > 0
                    wrn("Post rank\n");
                    debug_print_pop(DEBUG_PRINT_POP);
#endif // DEBUG_PRINT_POP

                    curr_pop_->select();

#if DEBUG_PRINT_POP > 0
                    wrn("Post select\n");
                    debug_print_pop(DEBUG_PRINT_POP);
#endif // DEBUG_PRINT_POP

                    // Instrumentations
                    const Candidate * best_candidate =
                        curr_pop_->candidates().front();
                    const Candidate * worst_candidate =
                        curr_pop_->candidates().front();

                    const float gen_best_score = best_candidate->get_score();
                    const ulong gen_best_len = best_candidate->nodes().size();
                    const float gen_worst_score = worst_candidate->get_score();
                    const double gen_time = ((get_timestamp_us() - genStartTime) / 1e3);
                    msg(gen_msg_fmt, i,
                        gen_best_score,
                        gen_best_score / gen_best_len,
                        gen_worst_score,
                        gen_time);

                    tot_gen_time += gen_time;

                    const int n_gens = i + 1;
                    msg(avg_time_msg_fmt,
                        (float) (POPULATION_COUNTERS.evolve_time) / n_gens,
                        (float) (POPULATION_COUNTERS.score_time) / n_gens,
                        (float) (POPULATION_COUNTERS.rank_time) / n_gens,
                        (float) (POPULATION_COUNTERS.select_time) / n_gens,
                        (float) tot_gen_time / n_gens);

                    // Exit loop if best score is low enough
                    if (gen_best_score < OPTIONS.score_stop_threshold) {
                        msg("Score stop threshold %.2f reached\n",
                            OPTIONS.score_stop_threshold);
                        break;
                    }
                    else {
                        // update best sols for this work area
                        for (int i = 0; i < OPTIONS.n_best_sols; i++)
                            best_sols_[wa_name][i] =
                                std::shared_ptr<Candidate>(curr_pop_->candidates().at(i)->clone());

                        if (float_approximates(gen_best_score, lastgen_best_score)) {
                            stagnant_count++;
                        }
                        else if (gen_best_score > lastgen_best_score) {
                            err("Best is worse than last gen!\n");
                            debug_print_pop(16);
                            die("Something is wrong...\n");
                        }
                        else {
                            stagnant_count = 0;
                        }

                        lastgen_best_score = gen_best_score;

                        if (stagnant_count >= OPTIONS.max_stagnant_gens) {
                            wrn("Solver stopped because max stagnancy is reached (%d)\n", OPTIONS.max_stagnant_gens);
                            break;
                        }
                        else {
                            msg("Current stagnancy: %d, max: %d\n", stagnant_count, OPTIONS.max_stagnant_gens);
                        }
                    }

                    swap_pop_buffs();
                }
            }
        }

        free(gen_msg_fmt);
        free(avg_time_msg_fmt);

        delete curr_pop_;
        curr_pop_ = nullptr;
        delete buff_pop_;
        buff_pop_ = nullptr;
    }

    this->print_end_msg();
}

} // namespace elfin
