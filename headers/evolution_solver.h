#ifndef EVOLUTIONSOLVER_H
#define EVOLUTIONSOLVER_H

#include <memory>

#include "population.h"

#define DEBUG_PRINT_POP 4

namespace elfin
{

typedef std::vector<std::shared_ptr<Candidate>> CandidateSharedPtrs;
typedef std::unordered_map<std::string,
        CandidateSharedPtrs> SolutionMap;

class EvolutionSolver
{
protected:
	/* data */
	SolutionMap best_sols_;
	double start_time_in_us_ = 0;

	/* modifiers */
	void collect_gen_data(
	    const Population & pop,
	    const size_t gen_id,
	    const double gen_start_time,
	    double & tot_gen_time,
	    size_t & stagnant_count,
	    float & lastgen_best_score,
	    CandidateSharedPtrs & best_sols,
	    bool & should_break);

	/* printers */
	void print_start_msg(const V3fList & shape) const;
	void print_end_msg() const;
	void print_timing() const;
	void debug_print_pop(
	    const Population & pop,
	    const size_t cutoff = DEBUG_PRINT_POP) const;

public:
	/* ctors */
	EvolutionSolver() {}

	/* dtors */

	/* accessors */
	const SolutionMap & best_sols() const { return best_sols_; }

	/* modifiers */
	void run();
};

} // namespace elfin

#endif /* include guard */
