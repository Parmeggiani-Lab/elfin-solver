#ifndef EVOLUTIONSOLVER_H
#define EVOLUTIONSOLVER_H

#include <memory>

#include "population.h"

namespace elfin {

/* types */
struct TestStat;

class EvolutionSolver {
private:
	/* types */
	struct PImpl;

	/* data */
	std::unique_ptr<PImpl> pimpl_;
public:
	/* ctors */
	EvolutionSolver(size_t const debug_pop_print_n = 4);

	/* dtors */
	virtual ~EvolutionSolver();

	/* modifiers */
	void run(WorkArea const& work_area, SolutionMaxHeap& output);

	/* tests */
	static TestStat test();
};

} // namespace elfin

#endif /* include guard */
