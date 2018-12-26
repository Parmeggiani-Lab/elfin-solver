#ifndef EVOLUTIONSOLVER_H
#define EVOLUTIONSOLVER_H

#include <memory>

#include "population.h"
#include "work_area.h"

#define DEBUG_PRINT_POP 4

namespace elfin {

/* types */
struct TestStat;
typedef std::vector<NodeTeamSP> NodeTeamSPList;
typedef std::unordered_map <std::string, NodeTeamSPList > SolutionMap;

class EvolutionSolver {
private:
	struct PImpl;
	std::unique_ptr<PImpl> p_impl_;

public:
	/* ctors */
	EvolutionSolver(size_t const debug_pop_print_n=4);

	/* dtors */
	virtual ~EvolutionSolver();

	/* accessors */
	bool has_result() const;
	SolutionMap const& best_sols() const;

	/* modifiers */
	void run();

	/* tests */
	static TestStat test();
};

} // namespace elfin

#endif /* include guard */
