#ifndef PATH_TEAM_H_
#define PATH_TEAM_H_

#include "node_team.h"
#include "node.h"
#include "test_data.h"

namespace elfin {

/* Fwd Decl */
struct TestStat;

// A PathTeam has either 0 or 2 tips at any given time.
class PathTeam : public NodeTeam {
private:
    /* type */
    class PImpl;

    /* data */
    std::unique_ptr<PImpl> pimpl_;

    /*modifiers */
    std::unique_ptr<PImpl> make_pimpl();
protected:
    /* data */
    std::unordered_map<NodeKey, NodeSP> nodes_;
    std::list<FreeChain> free_chains_;
    NodeKey scored_tip_ = nullptr;

    /* accessors */
    virtual PathTeam* virtual_clone() const;
    // Returns a tip node, may be random. For a hinged team, fixed nodes must
    // not be returned unlress there are no other nodes.
    // virtual NodeKey pick_tip_node() const;
    // Checks free_chains_.size() etc. are correct on entry to a mutation
    // method. Called before calling mutation methods from evolve().
    virtual void mutation_invariance_check() const;

    /* modifiers */
    virtual void virtual_copy(NodeTeam const& other);
    NodeKey add_member(ProtoModule const* const prot,
                       Transform const& tx = Transform());
    void remove_free_chains(NodeKey const node);
    virtual void calc_checksum();
    virtual void calc_score();
    // For testing: builds node team from recipe and returns the starting node.
    virtual NodeKey follow_recipe(tests::Recipe const& recipe,
                                  Transform const& shift_tx = Transform());
public:
    /* ctors */
    PathTeam(WorkArea const* wa);
    PathTeam(PathTeam const& other);
    PathTeam(PathTeam&& other);

    /* dtors */
    virtual ~PathTeam();

    /* accessors */
    virtual size_t size() const { return nodes_.size(); }
    PathGenerator gen_path() const;

    /* modifiers */
    PathTeam& operator=(PathTeam const& other);
    PathTeam& operator=(PathTeam && other);
    virtual void randomize();
    virtual mutation::Mode evolve(
        NodeTeam const& mother,
        NodeTeam const& father);
    void implement_recipe(tests::Recipe const& recipe,
                          Transform const& shift_tx = Transform());

    /* printers */
    virtual void print_to(std::ostream& os) const;
    virtual JSON to_json() const;

    /* tests */
    static TestStat test();
};  /* class PathTeam */

}  /* elfin */

#endif  /* end of include guard: PATH_TEAM_H_ */