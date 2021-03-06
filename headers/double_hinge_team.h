#ifndef DOUBLE_HINGE_TEAM_H_
#define DOUBLE_HINGE_TEAM_H_

#include "hinge_team.h"

namespace elfin {

class DoubleHingeTeam : public HingeTeam {
private:
    /* type */
    struct PImpl;

    /* data */
    std::unique_ptr<PImpl> pimpl_;
protected:
    /* data */
    UIJointKey hinge_ui_joint2_ = nullptr;

    /* accessors */
    virtual DoubleHingeTeam* virtual_clone() const;
    virtual void postprocess_json(JSON& json) const;

    /* modifiers */
    virtual void reset();
    virtual void virtual_copy(NodeTeam const& other);
    virtual void evaluate();
    virtual void virtual_implement_recipe(tests::Recipe const& recipe,
                                          FirstLastNodeKeyCallback const& postprocessor,
                                          Transform const& shift_tx);
public:
    /* ctors */
    DoubleHingeTeam(WorkArea const* const wa, uint32_t const seed);
    DoubleHingeTeam(DoubleHingeTeam const& other);
    DoubleHingeTeam(DoubleHingeTeam&& other);

    /* dtors */
    virtual ~DoubleHingeTeam();

    /* modifiers */
    DoubleHingeTeam& operator=(DoubleHingeTeam const& other);
    DoubleHingeTeam& operator=(DoubleHingeTeam && other);

    /* tests */
    static TestStat test();
};  /* class DoubleHingeTeam */

}  /* elfin */

#endif  /* end of include guard: DOUBLE_HINGE_TEAM_H_ */