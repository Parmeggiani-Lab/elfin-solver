#ifndef DOUBLE_HINGE_TEAM_H_
#define DOUBLE_HINGE_TEAM_H_

#include <array>

#include "hinge_team.h"

namespace elfin {

class DoubleHingeTeam : public HingeTeam {
private:
    /* type */
    class PImpl;

    /* data */
    std::unique_ptr<PImpl> pimpl_;

    /*modifiers */
    std::unique_ptr<PImpl> make_pimpl();
protected:
    /* accessors */
    virtual DoubleHingeTeam* virtual_clone() const;

    /* modifiers */
    virtual void reset();
    virtual void virtual_copy(NodeTeam const& other);
public:
    /* ctors */
    DoubleHingeTeam(WorkArea const* wa);
    DoubleHingeTeam(DoubleHingeTeam const& other);
    DoubleHingeTeam(DoubleHingeTeam&& other);

    /* dtors */
    virtual ~DoubleHingeTeam();

    /* tests */
    static TestStat test();
};  /* class DoubleHingeTeam */

}  /* elfin */

#endif  /* end of include guard: DOUBLE_HINGE_TEAM_H_ */