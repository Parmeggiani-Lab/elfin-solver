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

Chromosome & (Chromosome::*assign_chromo_funct)(Chromosome const&) = &Chromosome::operator=;
ulong (*get_dice_funct)(ulong) = getDice;
bool (Chromosome::*cross_chromosome_funct)(Chromosome const&, Chromosome&) const = &Chromosome::cross;
void (Chromosome::*auto_mutate_chromo_funct)() = &Chromosome::autoMutate;
void (Chromosome::*randomise_chromo_funct)() = &Chromosome::randomise;
bool (Chromosome::*point_mutate_chromo_funct)() = &Chromosome::pointMutate;
bool (Chromosome::*limb_mutate_chromo_funct)() = &Chromosome::limbMutate;

void
EvolutionSolver::set_length_guesses(const Points3f & shape) {
	/*
	 * Calculate expected length as sum of point
	 * displacements over avg pair module distance
	 */
	float sum_dist = 0.0f;
	for (auto i = shape.begin() + 1; i != shape.end(); ++i)
		sum_dist += (i - 1)->dist_to(*i);

	// Add one because division counts segments. We want number of points.
	int exp_len = 1 + round(sum_dist / options_.avg_pair_dist);

	Chromosome::setup(exp_len, options_.len_dev_alw, relamat_, radii_list_);
}

void
EvolutionSolver::init_pop_buffs(const WorkArea & wa)
{
	TIMING_START(startTimeInit);
	{
		Population * p = new Population(options_, wa);
		p->init(pop_size_, true);
		curr_pop_ = p;

		buff_pop_ = new Population(options_, wa);
		buff_pop_->init(pop_size_, false);
	}
	TIMING_END("init", startTimeInit);

	// We filled buffer first (because curr_pop_ shouldn't be modified)
	swap_pop_buffs();
}

// void
// EvolutionSolver::evolve_population()
// {
// #ifdef _VTUNE
// 	__itt_resume();  // start VTune, again use 2 underscores
// #endif

// 	CrossingVector possibleCrossings;

// 	TIMING_START(startTimeEvolving);
// 	{
// 		// Probabilistic evolution
// 		msg("Evolution: %.2f%%", (float) 0.0f);

// 		ulong cross_count = 0, pm_count = 0, lm_count = 0, rand_count = 0;
// 		const ulong gaPopBlock = options_.ga_pop_size / 10;
// 		ulong cross_fail_count = 0;

// 		OMP_PAR_FOR
// 		for (int i = surviver_cutoff_; i < options_.ga_pop_size; i++)
// 		{
// 			Chromosome & chromo_to_evolve = buff_pop_data_[i];
// 			const ulong evolution_dice = surviver_cutoff_ +
// 			                            get_dice_funct(non_surviver_count_);

// 			if (evolution_dice < cross_cutoff_)
// 			{
// 				long motherId, fatherId;
// 				if (get_dice_funct(2))
// 				{
// 					motherId = get_dice_funct(surviver_cutoff_);
// 					fatherId = get_dice_funct(options_.ga_pop_size);
// 				}
// 				else
// 				{
// 					motherId = get_dice_funct(options_.ga_pop_size);
// 					fatherId = get_dice_funct(surviver_cutoff_);
// 				}

// 				const Chromosome & mother = curr_pop_data_[motherId];
// 				const Chromosome & father = curr_pop_data_[fatherId];

// 				// Check compatibility
// 				if (!(mother.*cross_chromosome_funct)(father, chromo_to_evolve))
// 				{
// 					// Pick a random parent to inherit from and then mutate
// 					(chromo_to_evolve.*assign_chromo_funct)(mother);
// 					(chromo_to_evolve.*auto_mutate_chromo_funct)();
// 					cross_fail_count++;
// 				}
// 				cross_count++;
// 			}
// 			else
// 			{
// 				// Replicate a high ranking parent
// 				const ulong parentId = get_dice_funct(surviver_cutoff_);
// 				(chromo_to_evolve.*assign_chromo_funct)(curr_pop_data_[parentId]);

// 				if (evolution_dice < point_mutate_cutoff_)
// 				{
// 					if (!(chromo_to_evolve.*point_mutate_chromo_funct)())
// 						(chromo_to_evolve.*randomise_chromo_funct)();
// 					pm_count++;
// 				}
// 				else if (evolution_dice < limb_mutate_cutoff_)
// 				{
// 					if (!(chromo_to_evolve.*limb_mutate_chromo_funct)())
// 						(chromo_to_evolve.*randomise_chromo_funct)();
// 					lm_count++;
// 				}
// 				else
// 				{
// 					// Individuals not covered by specified mutation
// 					// rates undergo random destructive mutation
// 					(chromo_to_evolve.*randomise_chromo_funct)();
// 					rand_count++;
// 				}
// 			}
// #ifndef _TARGET_GPU
// 			if (i % gaPopBlock == 0)
// 			{
// 				ERASE_LINE();
// 				msg("Evolution: %.2f%%", (float) i / options_.ga_pop_size);
// 			}
// #endif
// 		}

// 		ERASE_LINE();
// 		msg("Evolution: 100%% Done\n");

// 		// Keep some actual counts to make sure the RNG is working
// 		// correctly
// 		dbg("Mutation rates: cross %.2f (fail=%d), pm %.2f, lm %.2f, rand %.2f, survivalCount: %d\n",
// 		    (float) cross_count / non_surviver_count_,
// 		    cross_fail_count,
// 		    (float) pm_count / non_surviver_count_,
// 		    (float) lm_count / non_surviver_count_,
// 		    (float) rand_count / non_surviver_count_,
// 		    surviver_cutoff_);
// 	}
// 	tot_evolve_time_ += TIMING_END("evolving", startTimeEvolving);

// #ifdef _VTUNE
// 	__itt_pause(); // stop VTune
// #endif
// }

// void (Chromosome::*scoreChromoFunct)(const Points3f &) = &Chromosome::score;

// void
// EvolutionSolver::score_population(const Points3f & shape)
// {
// 	TIMING_START(startTimeScoring);
// 	{
// 		msg("Scoring: 0%%");
// 		const ulong scoreBlock = options_.ga_pop_size / 10;

// 		OMP_PAR_FOR
// 		for (int i = 0; i < options_.ga_pop_size; i++)
// 		{
// 			(buff_pop_data_[i].*scoreChromoFunct)(shape);
// #ifndef _TARGET_GPU
// 			if (i % scoreBlock == 0)
// 			{
// 				ERASE_LINE();
// 				msg("Scoring: %.2f%%",
// 				    (float) i / options_.ga_pop_size);
// 			}
// #endif
// 		}
// 		ERASE_LINE();
// 		msg("Scoring: 100%% Done\n");
// 	}
// 	tot_score_time_ += TIMING_END("scoring", startTimeScoring);
// }

// void
// EvolutionSolver::rank_population()
// {
// 	// Sort population according to fitness
// 	// (low score = more fit)
// 	TIMING_START(rank_start_time);
// 	{
// 		std::sort(buff_pop_->begin(),
// 		          buff_pop_->end());
// 	}
// 	tot_rank_time_ += TIMING_END("ranking", rank_start_time);
// }

// void
// EvolutionSolver::select_parents()
// {
// 	TIMING_START(start_time_select_parents);
// 	{
// 		// Ensure variety within survivors using hashmap
// 		// and crc as key
// 		using CrcMap = std::unordered_map<Crc32, Chromosome>;
// 		CrcMap crcMap;
// 		ulong uniqueCount = 0;

// 		// We don't want parallelism here because
// 		// the loop must priotise low indexes
// 		for (int i = 0; i < buff_pop_->size(); i++)
// 		{
// 			const Crc32 crc = buff_pop_->at(i).checksum();
// 			if (crcMap.find(crc) == crcMap.end())
// 			{
// 				// This individual is a new one - record
// 				crcMap[crc] = buff_pop_->at(i);
// 				uniqueCount++;

// 				if (uniqueCount >= surviver_cutoff_)
// 					break;
// 			}
// 		}

// 		// Insert map-value-indexed individual back into population
// 		ulong popIndex = 0;
// 		for (CrcMap::iterator it = crcMap.begin(); it != crcMap.end(); ++it)
// 			buff_pop_->at(popIndex++) = it->second;

// 		// Sort survivors
// 		std::sort(buff_pop_->begin(),
// 		          buff_pop_->begin() + uniqueCount);
// 	}
// 	tot_select_time_ += TIMING_END("selecting", start_time_select_parents);
// }

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
	    Chromosome::min_len(),
	    Chromosome::max_len(),
	    shape.size());
	msg("Using deviation allowance: %d nodes\n", options_.len_dev_alw);

	// Want auto significant figure detection with streams
	std::ostringstream popsize_ss;
	if (pop_size_ > 1000)
		popsize_ss << (float) (pop_size_ / 1000.0f) << "k";
	else
		popsize_ss << pop_size_;

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
	    surviver_cutoff_,
	    cross_cutoff_,
	    point_mutate_cutoff_,
	    limb_mutate_cutoff_,
	    pop_size_ - limb_mutate_cutoff_);

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
EvolutionSolver::start_timer()
{
	start_time_in_us_ = get_timestamp_us();
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
	options_(options),
	pop_size_(options_.ga_pop_size)
{
	surviver_cutoff_ = std::round(options_.ga_survive_rate * pop_size_);

	non_surviver_count_ = (pop_size_ - surviver_cutoff_);

	cross_cutoff_ = surviver_cutoff_ +
	                std::round(options_.ga_cross_rate * non_surviver_count_);

	point_mutate_cutoff_ = cross_cutoff_ +
	                       std::round(options_.ga_point_mutate_rate * non_surviver_count_);

	limb_mutate_cutoff_ = std::min(
	                          (ulong) (point_mutate_cutoff_ + std::round(options_.ga_limb_mutate_rate * non_surviver_count_)),
	                          (ulong) pop_size_);
}

void
EvolutionSolver::run()
{
	this->start_timer();
	for (auto & itr : spec_.get_work_areas()) {
		const Points3f shape = itr.second.to_points3f();

		set_length_guesses(shape);

		this->print_start_msg(shape);

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
			err(ss.str().c_str());
			continue;
		}
		init_pop_buffs(wa);

		// init_population(shape);
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

		if (!options_.dry_run) {
			MAP_DATA()
			{
				for (int i = 0; i < options_.ga_iters; i++)
				{
					const double genStartTime = get_timestamp_us();
					{
						// evolve_population();
						buff_pop_->evolve(curr_pop_);

						// score_population(shape);
						buff_pop_->score();

						// rank_population();
						buff_pop_->rank();

						// select_parents();
						buff_pop_->select();

						swap_pop_buffs();
					}

					const Candidate * best_candidate = curr_pop_->candidates().front();
					const Candidate * worst_candidate = curr_pop_->candidates().front();

					const float gen_best_score = best_candidate->get_score();
					const ulong gen_best_len = best_candidate->nodes().size();
					const float gen_worst_score = worst_candidate->get_score();
					const double genTime = ((get_timestamp_us() - genStartTime) / 1e3);
					msg(gen_msg_fmt, i,
					    gen_best_score,
					    gen_best_score / gen_best_len,
					    gen_worst_score,
					    genTime);

					tot_gen_time_ += genTime;

					msg(avg_time_msg_fmt,
					    (float) tot_evolve_time_ / (i + 1),
					    (float) tot_score_time_ / (i + 1),
					    (float) tot_rank_time_ / (i + 1),
					    (float) tot_select_time_ / (i + 1),
					    (float) tot_gen_time_ / (i + 1));

					// Can stop loop if best score is low enough
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
