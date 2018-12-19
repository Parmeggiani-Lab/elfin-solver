#ifndef POPULATION_H_
#define POPULATION_H_

#include <vector>
#include <type_traits>

#include "node_team.h"
#include "work_area.h"

namespace elfin {

class Population {
protected:
    /* type */
    typedef std::vector<NodeTeamSP> NodeTeams;

    /* data */
    WorkArea const& work_area_;
    NodeTeams teams[2];
    NodeTeams* front_buffer_ = nullptr;
    NodeTeams const* back_buffer_ = nullptr;

    /* accessors */
    NodeTeamSP create_team() const;

public:
    /* ctors */
    Population(WorkArea const& work_area);

    /* accessors */
    NodeTeams const* front_buffer() const { return front_buffer_; }
    NodeTeams const* back_buffer() const { return back_buffer_; }

    /* modifiers */
    void evolve();
    void rank();
    void select();
    void swap_buffer();
};

}  /* elfin */

#endif  /* end of include guard: POPULATION_H_ */