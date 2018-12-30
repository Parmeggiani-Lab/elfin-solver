#ifndef HINGE_TEAM_H_
#define HINGE_TEAM_H_

#include "path_team.h"

namespace elfin {

class HingeTeam : public PathTeam {
private:
    /* type */
    class PImpl;

    /* data */
    std::unique_ptr<PImpl> p_impl_;

    /*modifiers */
    std::unique_ptr<PImpl> init_pimpl();
public:
    /* ctors */
    // HingeTeam(WorkArea const* wa);
    // // Recipe ctor is used for testing.
    // HingeTeam(WorkArea const* wa, tests::Recipe const& recipe);
    // HingeTeam(HingeTeam const& other);
    // HingeTeam(HingeTeam&& other);

    /* dtors */
    virtual ~HingeTeam();

};

}  /* elfin */

#endif  /* end of include guard: HINGE_TEAM_H_ */