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
    /* data */
    NodeKey hinge_ = nullptr;

    /* accessors */
    virtual HingeTeam* virtual_clone() const;
    virtual FreeChain const& pick_tip_chain() const;
    virtual void mutation_invariance_check() const;
    virtual void add_node_check(ProtoModule const* const prot) const;
    virtual bool can_modify(NodeKey const tip) const;

    /* modifiers */
    virtual void restart();
    virtual void virtual_copy(NodeTeam const& other);
    virtual void calc_checksum();
    virtual void calc_score();
    virtual NodeKey follow_recipe(tests::Recipe const& recipe,
                                  Transform const& shift_tx = Transform());

public:
    /* ctors */
    HingeTeam(WorkArea const* wa);
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