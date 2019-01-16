#ifndef HINGE_TEAM_H_
#define HINGE_TEAM_H_

#include "path_team.h"

namespace elfin {

/* Fwd Decl */
struct UIJoint;
typedef UIJoint const* UIJointKey;

// A HingeTeam has one module already fixed by the user (or from hub
// assignment).
class HingeTeam : public PathTeam {
private:
    /* type */
    struct PImpl;

    /* data */
    std::unique_ptr<PImpl> pimpl_;

    /*modifiers */
    std::unique_ptr<PImpl> make_pimpl();
protected:
    /* data */
    UIJointKey hinge_ui_joint_ = nullptr;
    NodeKey hinge_ = nullptr;

    /* accessors */
    virtual HingeTeam* virtual_clone() const;
    virtual FreeTerm get_mutable_term() const;
    virtual NodeKey get_tip(bool const mutable_hint) const;
    virtual void mutation_invariance_check() const;
    virtual bool is_mutable(NodeKey const tip) const;
    virtual void postprocess_json(JSON& output) const;

    /* modifiers */
    virtual void reset();
    virtual void virtual_copy(NodeTeam const& other);
    virtual void calc_checksum();
    virtual void calc_score();
    virtual void virtual_implement_recipe(tests::Recipe const& recipe,
                                          FirstLastNodeKeyCallback const& postprocessor,
                                          Transform const& shift_tx);

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