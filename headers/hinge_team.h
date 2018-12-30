#ifndef HINGE_TEAM_H_
#define HINGE_TEAM_H_

#include "path_team.h"

namespace elfin {

class HingeTeam : public PathTeam {
private:
    /* type */
    class PImpl;

    /* data */
    std::unique_ptr<PImpl> pimpl_;

    /*modifiers */
    std::unique_ptr<PImpl> make_pimpl();
protected:
    /* accessors */
    virtual HingeTeam* virtual_clone() const;

    /* modifiers */
    virtual void virtual_copy(NodeTeam const& other);
    virtual void calc_score();

public:
    /* ctors */
    HingeTeam(WorkArea const* wa);
    // Recipe ctor is used for testing.
    HingeTeam(WorkArea const* wa, tests::Recipe const& recipe);
    HingeTeam(HingeTeam const& other);
    HingeTeam(HingeTeam&& other);

    /* dtors */
    virtual ~HingeTeam();

    /* modifiers */
    HingeTeam& operator=(HingeTeam const& other);
    HingeTeam& operator=(HingeTeam && other);

    /* tests */
    static TestStat test();
};

}  /* elfin */

#endif  /* end of include guard: HINGE_TEAM_H_ */