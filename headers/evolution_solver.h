#ifndef EVOLUTIONSOLVER_H
#define EVOLUTIONSOLVER_H

#include <memory>

#include "population.h"
#include "work_area.h"
#include "move_heap.h"

#define DEBUG_PRINT_POP 4

namespace elfin {

/* types */
struct TestStat;

// A MAX score heap (worst solutions at the top).
typedef MoveHeap<NodeTeamSP,
        std::vector<NodeTeamSP>,
        NodeTeam::SPLess> TeamSPMinHeap;
typedef std::priority_queue<NodeTeam const*,
        std::vector<NodeTeam const*>,
        NodeTeam::PtrGreater> TeamPtrMaxHeap;
typedef std::unordered_map <std::string,
        TeamSPMinHeap > SolutionMap;

class EvolutionSolver {
private:
	struct PImpl;
	std::unique_ptr<PImpl> pimpl_;

public:
	/* ctors */
	EvolutionSolver(size_t const debug_pop_print_n = 4);

	/* dtors */
	virtual ~EvolutionSolver();

	/* accessors */
	bool has_result() const;
	TeamPtrMaxHeap best_sols(std::string const& wa_name) const;

	/* modifiers */
	void run();

	/* tests */
	static TestStat test();
};

} // namespace elfin

#endif /* include guard */
