#ifndef DOUBLE_HINGE_TEAM_H_
#define DOUBLE_HINGE_TEAM_H_

#include "node_team.h"
#include "path_generator.h"
#include "recipe.h"

namespace elfin {

// A DoubleHingeTeam consists of two single-hinge HingeTeams. A 2-Hinge case
// is solved by randomly splitting the path between the 2 hinges, and solving
// each half using one HingeTeam.
class DoubleHingeTeam : public NodeTeam {
private:
    /* type */
    class PImpl;

    /* data */
    std::unique_ptr<PImpl> pimpl_;

    /*modifiers */
    std::unique_ptr<PImpl> make_pimpl();
protected:
    /* accessors */
    // virtual DoubleHingeTeam* virtual_clone() const;

    /* modifiers */
    // virtual void virtual_copy(NodeTeam const& other);
public:
    /* ctors */
    DoubleHingeTeam(WorkArea const* wa);
    // DoubleHingeTeam(DoubleHingeTeam const& other);
    // DoubleHingeTeam(DoubleHingeTeam&& other);

    /* dtors */
    virtual ~DoubleHingeTeam();

    /* accessors */
    // virtual size_t size() const;
    // PathGenerator gen_path() const;

    /* modifiers */
    // DoubleHingeTeam& operator=(DoubleHingeTeam const& other);
    // DoubleHingeTeam& operator=(DoubleHingeTeam && other);
    // virtual void randomize();
    // virtual mutation::Mode evolve(NodeTeam const& mother,
    //                               NodeTeam const& father);
    // For testing: builds node team from recipe and returns the starting node.
    // void implement_recipe(tests::Recipe const& recipe,
    //                       Transform const& shift_tx = Transform());

    /* printers */
    // virtual void print_to(std::ostream& os) const;
    // virtual JSON to_json() const;

    /* tests */
    static TestStat test();
};  /* class DoubleHingeTeam */

}  /* elfin */

#endif  /* end of include guard: DOUBLE_HINGE_TEAM_H_ */