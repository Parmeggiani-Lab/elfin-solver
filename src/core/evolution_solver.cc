#include "evolution_solver.h"

#include <cmath>
#include <sstream>
#include <algorithm>
#include <stdlib.h>
#include <unordered_map>
#include <limits>

#include "jutil.h"
#include "ParallelUtils.h"

namespace elfin
{

/* Private Methods */

#ifdef _VTUNE
#include <ittnotify.h>
#endif

Chromosome & (Chromosome::*assignChromoFunct)(Chromosome const&) = &Chromosome::operator=;
ulong (*getDiceFunct)(ulong) = getDice;
bool (Chromosome::*crossChromosomeFunct)(Chromosome const&, Chromosome&) const = &Chromosome::cross;
// Chromosome (Chromosome::*mutateChildChromoFunct)() const = &Chromosome::mutateChild;
void (Chromosome::*autoMutateChromoFunct)() = &Chromosome::autoMutate;
void (Chromosome::*randomiseChromoFunct)() = &Chromosome::randomise;
bool (Chromosome::*pointMutateChromoFunct)() = &Chromosome::pointMutate;
bool (Chromosome::*limbMutateChromoFunct)() = &Chromosome::limbMutate;

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
EvolutionSolver::evolve_population()
{
#ifdef _VTUNE
	__itt_resume();  // start VTune, again use 2 underscores
#endif

	CrossingVector possibleCrossings;

	TIMING_START(startTimeEvolving);
	{
		// Probabilistic evolution
		msg("Evolution: %.2f%%", (float) 0.0f);

		ulong cross_count = 0, pm_count = 0, lm_count = 0, rand_count = 0;
		const ulong gaPopBlock = options_.ga_pop_size / 10;
		ulong cross_fail_count = 0;

		OMP_PAR_FOR
		for (int i = surviver_cutoff_; i < options_.ga_pop_size; i++)
		{
			Chromosome & chromoToEvolve = buff_pop_data_[i];
			const ulong evolutionDice = surviver_cutoff_ +
			                            getDiceFunct(non_surviver_count_);

			if (evolutionDice < cross_cutoff_)
			{
				long motherId, fatherId;
				if (getDiceFunct(2))
				{
					motherId = getDiceFunct(surviver_cutoff_);
					fatherId = getDiceFunct(options_.ga_pop_size);
				}
				else
				{
					motherId = getDiceFunct(options_.ga_pop_size);
					fatherId = getDiceFunct(surviver_cutoff_);
				}

				const Chromosome & mother = curr_pop_data_[motherId];
				const Chromosome & father = curr_pop_data_[fatherId];

				// Check compatibility
				if (!(mother.*crossChromosomeFunct)(father, chromoToEvolve))
				{
					// Pick a random parent to inherit from and then mutate
					// (chromoToEvolve.*assignChromoFunct)((mother.*mutateChildChromoFunct)());
					(chromoToEvolve.*assignChromoFunct)(mother);
					(chromoToEvolve.*autoMutateChromoFunct)();
					cross_fail_count++;
				}
				cross_count++;
			}
			else
			{
				// Replicate a high ranking parent
				const ulong parentId = getDiceFunct(surviver_cutoff_);
				(chromoToEvolve.*assignChromoFunct)(curr_pop_data_[parentId]);

				if (evolutionDice < point_mutate_cutoff_)
				{
					if (!(chromoToEvolve.*pointMutateChromoFunct)())
						(chromoToEvolve.*randomiseChromoFunct)();
					pm_count++;
				}
				else if (evolutionDice < limb_mutate_cutoff_)
				{
					if (!(chromoToEvolve.*limbMutateChromoFunct)())
						(chromoToEvolve.*randomiseChromoFunct)();
					lm_count++;
				}
				else
				{
					// Individuals not covered by specified mutation
					// rates undergo random destructive mutation
					(chromoToEvolve.*randomiseChromoFunct)();
					rand_count++;
				}
			}
#ifndef _TARGET_GPU
			if (i % gaPopBlock == 0)
			{
				ERASE_LINE();
				msg("Evolution: %.2f%%", (float) i / options_.ga_pop_size);
			}
#endif
		}

		ERASE_LINE();
		msg("Evolution: 100%% Done\n");

		// Keep some actual counts to make sure the RNG is working
		// correctly
		dbg("Mutation rates: cross %.2f (fail=%d), pm %.2f, lm %.2f, rand %.2f, survivalCount: %d\n",
		    (float) cross_count / non_surviver_count_,
		    cross_fail_count,
		    (float) pm_count / non_surviver_count_,
		    (float) lm_count / non_surviver_count_,
		    (float) rand_count / non_surviver_count_,
		    surviver_cutoff_);
	}
	tot_evolve_time_ += TIMING_END("evolving", startTimeEvolving);

#ifdef _VTUNE
	__itt_pause(); // stop VTune
#endif
}

void (Chromosome::*scoreChromoFunct)(const Points3f &) = &Chromosome::score;

void
EvolutionSolver::score_population(const Points3f & shape)
{
	TIMING_START(startTimeScoring);
	{
		msg("Scoring: 0%%");
		const ulong scoreBlock = options_.ga_pop_size / 10;

		OMP_PAR_FOR
		for (int i = 0; i < options_.ga_pop_size; i++)
		{
			(buff_pop_data_[i].*scoreChromoFunct)(shape);
#ifndef _TARGET_GPU
			if (i % scoreBlock == 0)
			{
				ERASE_LINE();
				msg("Scoring: %.2f%%",
				    (float) i / options_.ga_pop_size);
			}
#endif
		}
		ERASE_LINE();
		msg("Scoring: 100%% Done\n");
	}
	tot_score_time_ += TIMING_END("scoring", startTimeScoring);
}

void
EvolutionSolver::rank_population()
{
	// Sort population according to fitness
	// (low score = more fit)
	TIMING_START(rank_start_time);
	{
		std::sort(buff_pop_->begin(),
		          buff_pop_->end());
	}
	tot_rank_time_ += TIMING_END("ranking", rank_start_time);
}

void
EvolutionSolver::select_parents()
{
	TIMING_START(start_time_select_parents);
	{
		// Ensure variety within survivors using hashmap
		// and crc as key
		using CrcMap = std::unordered_map<Crc32, Chromosome>;
		CrcMap crcMap;
		ulong uniqueCount = 0;

		// We don't want parallelism here because
		// the loop must priotise low indexes
		for (int i = 0; i < buff_pop_->size(); i++)
		{
			const Crc32 crc = buff_pop_->at(i).checksum();
			if (crcMap.find(crc) == crcMap.end())
			{
				// This individual is a new one - record
				crcMap[crc] = buff_pop_->at(i);
				uniqueCount++;

				if (uniqueCount >= surviver_cutoff_)
					break;
			}
		}

		// Insert map-value-indexed individual back into population
		ulong popIndex = 0;
		for (CrcMap::iterator it = crcMap.begin(); it != crcMap.end(); ++it)
			buff_pop_->at(popIndex++) = it->second;

		// Sort survivors
		std::sort(buff_pop_->begin(),
		          buff_pop_->begin() + uniqueCount);
	}
	tot_select_time_ += TIMING_END("selecting", start_time_select_parents);
}

void
EvolutionSolver::swap_pop_buffers()
{
	#pragma omp single
	{
		const Population * tmp = curr_pop_;
		curr_pop_ = buff_pop_;
		buff_pop_ = const_cast<Population *>(tmp);
		buff_pop_data_ = buff_pop_->data();
		curr_pop_data_ = curr_pop_->data();
	}
}

void
EvolutionSolver::init_population(const Points3f & shape)
{
	TIMING_START(startTimeInit);
	{
		population_buffers_[0] = Population();
		population_buffers_[0].resize(options_.ga_pop_size);
		population_buffers_[1] = Population();
		population_buffers_[1].resize(options_.ga_pop_size);
		curr_pop_ = &(population_buffers_[0]);
		buff_pop_ = &(population_buffers_[1]);

		if (buff_pop_->size() != curr_pop_->size() || buff_pop_->size() == 0)
			die("Buffer size (%d) and current population size (%d) differ or is zero.\n",
			    buff_pop_->size(), curr_pop_->size());

		pop_size_ = options_.ga_pop_size;

		buff_pop_data_ = buff_pop_->data();
		curr_pop_data_ = curr_pop_->data();

		const ulong block = options_.ga_pop_size / 10;

		msg("Initialising population: %.2f%%", 0.0f);

		Chromosome * mpb0 = population_buffers_[0].data();
		Chromosome * mpb1 = population_buffers_[1].data();

		OMP_PAR_FOR
		for (int i = 0; i < options_.ga_pop_size; i++)
		{
			(mpb0[i].*randomiseChromoFunct)();
			(mpb1[i].*assignChromoFunct)(mpb0[i]);
#ifndef _TARGET_GPU
			if (i % block == 0)
			{
				ERASE_LINE();
				msg("Initialising population: %.2f%%", (float) i / options_.ga_pop_size);
			}
#endif
		}

		// Hard test - ensure scoring last element is OK
		population_buffers_[0].at(options_.ga_pop_size - 1).score(shape);
		population_buffers_[1].at(options_.ga_pop_size - 1).score(shape);

		ERASE_LINE();
		msg("Initialising population: 100%%\n");

	}
	TIMING_END("init", startTimeInit);

	// We filled buffer first (because curr_pop_ shouldn't be modified)
	swap_pop_buffers();
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
	if (options_.ga_pop_size > 1000)
		popsize_ss << (float) (options_.ga_pop_size / 1000.0f) << "k";
	else
		popsize_ss << options_.ga_pop_size;

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
	    options_.ga_pop_size - limb_mutate_cutoff_);

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

	for (int i = 0; i < options_.n_best_sols; i++)
	{
		const auto & p = curr_pop_->at(i);
		msg("Solution #%d score %.2f: \n%s\n",
		    p.getScore(),
		    i,
		    p.to_string().c_str());
	}
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
	options_(options)
{
	surviver_cutoff_ = std::round(options_.ga_survive_rate * options_.ga_pop_size);

	non_surviver_count_ = (options_.ga_pop_size - surviver_cutoff_);

	cross_cutoff_ = surviver_cutoff_ +
	                std::round(options_.ga_cross_rate * non_surviver_count_);

	point_mutate_cutoff_ = cross_cutoff_ +
	                       std::round(options_.ga_point_mutate_rate * non_surviver_count_);

	limb_mutate_cutoff_ = std::min(
	                          (ulong) (point_mutate_cutoff_ + std::round(options_.ga_limb_mutate_rate * non_surviver_count_)),
	                          (ulong) options_.ga_pop_size);
}

void
EvolutionSolver::run()
{
	this->start_timer();
	for (auto & itr : spec_.get_work_areas()) {
		const Points3f shape = itr.second.to_points3f();

		set_length_guesses(shape);

		this->print_start_msg(shape);

		init_population(shape);

		best_so_far_.resize(options_.n_best_sols);

		float lastGenBestScore = std::numeric_limits<float>::infinity();
		uint stagnantCount = 0;

		const int genDispDigits = std::ceil(std::log(options_.ga_iters) / std::log(10));
		char * genMsgFmt;
		int asprintf_ret = -1;
		asprintf_ret = asprintf(&genMsgFmt,
		                        "Generation #%%%dd: best=%%.2f (%%.2f/module), worst=%%.2f, time taken=%%.0fms\n", genDispDigits);
		char * avgTimeMsgFmt;
		asprintf_ret = asprintf(&avgTimeMsgFmt,
		                        "Avg Times: Evolve=%%.0f,Score=%%.0f,Rank=%%.0f,Select=%%.0f,Gen=%%.0f\n");


		if (!options_.dry_run) {
			MAP_DATA()
			{
				for (int i = 0; i < options_.ga_iters; i++)
				{
					const double genStartTime = get_timestamp_us();
					{
						evolve_population();

						score_population(shape);

						rank_population();

						select_parents();

						swap_pop_buffers();
					}

					const float genBestScore = curr_pop_->front().getScore();
					const ulong genBestChromoLen = curr_pop_->front().genes().size();
					const float genWorstScore = curr_pop_->back().getScore();
					const double genTime = ((get_timestamp_us() - genStartTime) / 1e3);
					msg(genMsgFmt, i,
					    genBestScore,
					    genBestScore / genBestChromoLen,
					    genWorstScore,
					    genTime);

					tot_gen_time_ += genTime;

					msg(avgTimeMsgFmt,
					    (float) tot_evolve_time_ / (i + 1),
					    (float) tot_score_time_ / (i + 1),
					    (float) tot_rank_time_ / (i + 1),
					    (float) tot_select_time_ / (i + 1),
					    (float) tot_gen_time_ / (i + 1));

					// Can stop loop if best score is low enough
					if (genBestScore < options_.score_stop_threshold)
					{
						msg("Score stop threshold %.2f reached\n", options_.score_stop_threshold);
						break;
					}
					else
					{
						for (int i = 0; i < options_.n_best_sols; i++)
							best_so_far_.at(i) = curr_pop_->at(i);

						if (float_approximates(genBestScore, lastGenBestScore))
						{
							stagnantCount++;
						}
						else
						{
							stagnantCount = 0;
						}

						lastGenBestScore = genBestScore;

						if (stagnantCount >= options_.max_stagnant_gens)
						{
							wrn("Solver stopped because max stagnancy is reached (%d)\n", options_.max_stagnant_gens);
							break;
						}
						else
						{
							msg("Current stagnancy: %d, max: %d\n", stagnantCount, options_.max_stagnant_gens);
						}
					}
				}
			}
		}

		free(genMsgFmt);
		free(avgTimeMsgFmt);
	}

	this->print_end_msg();
}

} // namespace elfin
