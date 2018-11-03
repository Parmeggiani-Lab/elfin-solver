#include "evolution_solver.h"

#include <cmath>
#include <sstream>
#include <algorithm>
#include <stdlib.h>
#include <unordered_map>
#include <limits>

#include "jutil.h"
#include "parallel_utils.h"

namespace elfin
{

/* Private Methods */

#ifdef _VTUNE
#include <ittnotify.h>
#endif

ulong (*get_dice_funct)(ulong) = getDice;

void
EvolutionSolver::init_pop_buffs(const WorkArea & wa)
{
	TIMING_START(startTimeInit);
	{
		Population * p = new Population(options_, wa);
		p->init(options_.ga_pop_size, true);
		curr_pop_ = p;

		buff_pop_ = new Population(options_, wa);
		buff_pop_->init(options_.ga_pop_size, false);
	}
	TIMING_END("init", startTimeInit);

	// We filled buffer first (because curr_pop_ shouldn't be modified)
	swap_pop_buffs();
}

void
EvolutionSolver::swap_pop_buffs()
{
	#pragma omp single
	{
		const Population * tmp = curr_pop_;
		curr_pop_ = buff_pop_;
		buff_pop_ = const_cast<Population *>(tmp);
	}
}

void
EvolutionSolver::print_start_msg(const Points3f & shape)
{
	for (auto & p : shape)
		dbg("Work Area Point: %s\n", p.to_string().c_str());

	msg("Length guess: %u~%u, spec has %d points\n",
	    curr_pop_->counts().len.min,
	    curr_pop_->counts().len.max,
	    shape.size());
	msg("Using deviation allowance: %d nodes\n", options_.len_dev_alw);

	// Want auto significant figure detection with streams
	std::ostringstream popsize_ss;
	const ulong pop_size = options_.ga_pop_size;
	if (pop_size > 1000)
		popsize_ss << (float) (pop_size / 1000.0f) << "k";
	else
		popsize_ss << pop_size;

	std::ostringstream nitr_ss;
	if (options_.ga_iters > 1000)
		nitr_ss << (float) (options_.ga_iters / 1000.0f) << "k";
	else
		nitr_ss << options_.ga_iters;


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
	    curr_pop_->counts().survivors,
	    curr_pop_->mutation_cutoffs().cross,
	    curr_pop_->mutation_cutoffs().point,
	    curr_pop_->mutation_cutoffs().limb,
	    pop_size - curr_pop_->mutation_cutoffs().limb);

	const int n_omp_devices = omp_get_num_devices();
	const int host_device_id = omp_get_initial_device();
	msg("There are %d devices. Host is #%d; currently using #%d\n", n_omp_devices, host_device_id, options_.device);
	omp_set_default_device(options_.device);

	#pragma omp parallel
	{
		if (omp_get_thread_num() == 0)
			msg("Running with %d threads\n", omp_get_max_threads());
	}
}

void
EvolutionSolver::print_end_msg()
{
	msg("EvolutionSolver finished: ");
	this->print_timing();
}

void
EvolutionSolver::print_timing()
{
	const double time_elapsed_in_us = get_timestamp_us() - start_time_in_us_;
	const uint64_t minutes = std::floor(time_elapsed_in_us / 1e6 / 60.0f);
	const uint64_t seconds = std::floor(fmod(time_elapsed_in_us / 1e6, 60.0f));
	const uint64_t milliseconds = std::floor(fmod(time_elapsed_in_us / 1e3, 1000.0f));
	raw("%um %us %ums\n",
	    minutes, seconds, milliseconds);
}

/* Public Methods */

EvolutionSolver::EvolutionSolver(const RelaMat & relamat,
                                 const Spec & spec,
                                 const RadiiList & radii_list,
                                 const Options & options) :
	relamat_(relamat),
	spec_(spec),
	radii_list_(radii_list),
	options_(options)
{
}

void
EvolutionSolver::run()
{
	start_time_in_us_ = get_timestamp_us();
	for (auto & itr : spec_.get_work_areas()) {

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
		init_pop_buffs(wa);

		wrn("Here!!\n");
		const Points3f shape = itr.second.to_points3f();
		this->print_start_msg(shape);

		best_sols_[wa_name] = std::vector<Candidate *>();
		best_sols_[wa_name].resize(options_.n_best_sols);

		float lastgen_best_score = std::numeric_limits<float>::infinity();
		uint stagnant_count = 0;

		const int genDispDigits = std::ceil(std::log(options_.ga_iters) / std::log(10));
		char * gen_msg_fmt;
		int asprintf_ret = -1;
		asprintf_ret = asprintf(&gen_msg_fmt,
		                        "Generation #%%%dd: best=%%.2f (%%.2f/module), worst=%%.2f, time taken=%%.0fms\n", genDispDigits);
		char * avg_time_msg_fmt;
		asprintf_ret = asprintf(&avg_time_msg_fmt,
		                        "Avg Times: Evolve=%%.0f,Score=%%.0f,Rank=%%.0f,Select=%%.0f,Gen=%%.0f\n");

		double tot_gen_time = 0.0f;
		if (!options_.dry_run) {
			MAP_DATA()
			{
				for (int i = 0; i < options_.ga_iters; i++)
				{
					const double genStartTime = get_timestamp_us();
					{
						buff_pop_->evolve(curr_pop_);
						buff_pop_->score();
						buff_pop_->rank();
						buff_pop_->select();
						swap_pop_buffs();
					}

					// Instrumentations
					const Candidate * best_candidate = curr_pop_->candidates().front();
					const Candidate * worst_candidate = curr_pop_->candidates().front();

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
					    (float) (curr_pop_->counts().evolve_time +
					             buff_pop_->counts().evolve_time) / n_gens,
					    (float) (curr_pop_->counts().score_time +
					             buff_pop_->counts().score_time) / n_gens,
					    (float) (curr_pop_->counts().rank_time +
					             buff_pop_->counts().rank_time) / n_gens,
					    (float) (curr_pop_->counts().select_time +
					             buff_pop_->counts().select_time) / n_gens,
					    (float) tot_gen_time / n_gens);

					// Exit loop if best score is low enough
					if (gen_best_score < options_.score_stop_threshold)
					{
						msg("Score stop threshold %.2f reached\n", options_.score_stop_threshold);
						break;
					}
					else
					{
						// update best sols for this work area
						for (int i = 0; i < options_.n_best_sols; i++)
							best_sols_[wa_name][i] = curr_pop_->candidates().at(i);

						if (float_approximates(gen_best_score, lastgen_best_score))
						{
							stagnant_count++;
						}
						else
						{
							stagnant_count = 0;
						}

						lastgen_best_score = gen_best_score;

						if (stagnant_count >= options_.max_stagnant_gens)
						{
							wrn("Solver stopped because max stagnancy is reached (%d)\n", options_.max_stagnant_gens);
							break;
						}
						else
						{
							msg("Current stagnancy: %d, max: %d\n", stagnant_count, options_.max_stagnant_gens);
						}
					}
				}
			}
		}

		free(gen_msg_fmt);
		free(avg_time_msg_fmt);

		delete curr_pop_;
		delete buff_pop_;
	}

	this->print_end_msg();
}

} // namespace elfin
