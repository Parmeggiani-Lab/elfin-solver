#ifndef PATH_TEAM_H_
#define PATH_TEAM_H_

#include <functional>

#include "node_team.h"
#include "node.h"
#include "recipe.h"

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
    /* types */
    typedef std::function<void(NodeKey)> NodeKeyCallback;

    /* data */
    std::unordered_map<NodeKey, NodeSP> nodes_;
    std::list<FreeTerm> free_terms_;
    V3fList const* scored_path_ = nullptr;
    NodeKeyMap nk_map_;

    /* accessors */
    virtual PathTeam* virtual_clone() const;
    virtual FreeTerm get_mutable_chain() const;
    virtual NodeKey get_tip(bool const mutable_hint) const;
    virtual void mutation_invariance_check() const;
    virtual bool is_mutable(NodeKey const nk) const;

    /* modifiers */
    virtual void reset();
    virtual void virtual_copy(NodeTeam const& other);
    NodeKey add_node(ProtoModule const* const prot,
                     Transform const& tx = Transform(),
                     bool const innert = false,
                     FreeTerm const* const exclude_ft = nullptr);
    void remove_free_terms(NodeKey const node);
    virtual void calc_checksum();
    virtual void calc_score();
    Node* get_node(NodeKey const nk);
    // For testing: builds node team from recipe and returns the starting node.
    virtual void virtual_implement_recipe(tests::Recipe const& recipe,
                                          NodeKeyCallback const& cb_on_first_node,
                                          Transform const& shift_tx);
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
    virtual mutation::Mode evolve(NodeTeam const& mother,
                                  NodeTeam const& father);
    void implement_recipe(tests::Recipe const& recipe,
                          Transform const& shift_tx = Transform()) {
        virtual_implement_recipe(recipe, NodeKeyCallback(), shift_tx);
    }

    /* printers */
    virtual void print_to(std::ostream& os) const;
    virtual JSON to_json() const;

    /* tests */
    static TestStat test();
};  /* class PathTeam */

}  /* elfin */

#endif  /* end of include guard: PATH_TEAM_H_ */