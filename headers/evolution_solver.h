#ifndef EVOLUTIONSOLVER_H
#define EVOLUTIONSOLVER_H

#include "shorthands.h"
#include "options.h"
#include "spec.h"
#include "population.h"
#include "radii.h"

// #include "Chromosome.h"

namespace elfin
{

typedef std::unordered_map<std::string, std::vector<Candidate *>> SolutionMap;

class EvolutionSolver
{
protected:
	const RelaMat & relamat_;
	const Spec & spec_;
	const RadiiList & radii_list_;
	const Options & options_;
	SolutionMap best_sols_;

	double start_time_in_us_ = 0;

	const Population * curr_pop_;
	Population * buff_pop_;

	void set_length_guesses(const Points3f & shape);

	void init_pop_buffs(const WorkArea & wa);
	void swap_pop_buffs();

	void print_start_msg(const Points3f & shape);
	void print_end_msg();
	void print_timing();

public:
	EvolutionSolver(const RelaMat & relaMat,
	                const Spec & spec,
	                const RadiiList & radiiList,
	                const Options & options);

	const WorkAreas & work_areas() const { return spec_.get_work_areas(); }
	const SolutionMap & best_sols() const { return best_sols_; }

	void run();
};

} // namespace elfin

#endif /* include guard */
