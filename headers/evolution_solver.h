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
	/* data members */
	SolutionMap best_sols_;
	double start_time_in_us_ = 0;
	Population * curr_pop_;
	const Population * buff_pop_;

	/* other methods */
	void set_length_guesses(const V3fList & shape);

	void init_pop_buffs(const WorkArea & wa);
	void swap_pop_buffs();

	void collect_gen_data(
	    const size_t gen_id,
	    const double gen_start_time,
	    double & tot_gen_time,
	    size_t & stagnant_count,
	    float & lastgen_best_score,
	    CandidateSharedPtrs & best_sols,
	    bool & should_break);

	void print_start_msg(const V3fList & shape) const;
	void print_end_msg() const;
	void print_timing() const;

	void debug_print_pop(size_t cutoff = -1) const;

public:
	/* ctors & dtors */
	// EvolutionSolver();
	virtual ~EvolutionSolver();

	/* getters & setters */
	const SolutionMap & best_sols() const { return best_sols_; }

	/* other methods */
	/*
	 * Executes the genetic algorithm solver.
	 */
	void run();
};

} // namespace elfin

#endif /* include guard */
