#ifndef EVOLUTIONSOLVER_H
#define EVOLUTIONSOLVER_H

#include <memory>

#include "population.h"

namespace elfin
{

typedef std::vector<std::shared_ptr<Candidate>> CandidateSharedPtrs;
typedef std::unordered_map<std::string,
        CandidateSharedPtrs> SolutionMap;

class EvolutionSolver
{
protected:
	SolutionMap best_sols_;

	double start_time_in_us_ = 0;

	Population * curr_pop_;
	const Population * buff_pop_;

	void set_length_guesses(const Points3f & shape);

	void init_pop_buffs(const WorkArea & wa);
	void swap_pop_buffs();

	void print_start_msg(const Points3f & shape);
	void print_end_msg();
	void print_timing();

public:
	const SolutionMap & best_sols() const { return best_sols_; }

	void run();
};

} // namespace elfin

#endif /* include guard */
