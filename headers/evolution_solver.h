#ifndef EVOLUTIONSOLVER_H
#define EVOLUTIONSOLVER_H

#include <memory>

#include "population.h"
#include "work_area.h"

#define DEBUG_PRINT_POP 4

namespace elfin {

typedef std::vector<NodeTeamSP> NodeTeamSPList;
typedef std::unordered_map <std::string, NodeTeamSPList > SolutionMap;

class EvolutionSolver {
protected:
	/* data */
	bool has_result_ = false;
	SolutionMap best_sols_;
	double start_time_in_us_ = 0;

	/* modifiers */
	void collect_gen_data(
	    Population const& pop,
	    size_t const gen_id,
	    double const gen_start_time,
	    double& tot_gen_time,
	    size_t& stagnant_count,
	    float& lastgen_best_score,
	    NodeTeamSPList& best_sols,
	    bool& should_break);

	/* printers */
	void print_start_msg(WorkArea const& wa) const;
	void print_end_msg() const;
	void print_timing() const;
	void debug_print_pop(
	    Population const& pop,
	    size_t const cutoff = DEBUG_PRINT_POP) const;

public:
	/* accessors */
	bool has_result() const { return has_result_; }
	SolutionMap const& best_sols() const { return best_sols_; }

	/* modifiers */
	void run();
};

} // namespace elfin

#endif /* include guard */
