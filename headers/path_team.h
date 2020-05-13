#ifndef PATH_TEAM_H_
#define PATH_TEAM_H_

#include <functional>

#include "node_team.h"
#include "node.h"
#include "recipe.h"
#include "path_generator.h"
#include "work_area.h"

namespace elfin {

/* Fwd Decl */
struct TestStat;

// A PathTeam has either 0 or 2 tips at any given time.
class PathTeam : public NodeTeam {
private:
    /* type */
    struct PImpl;

    /* data */
    std::unique_ptr<PImpl> pimpl_;

    /*modifiers */
    std::unique_ptr<PImpl> make_pimpl();
protected:
    /* types */
    typedef std::function<void(NodeKey const first_node, NodeKey const last_node)> FirstLastNodeKeyCallback;

    /* data */
    std::unordered_map<NodeKey, NodeSP> nodes_;
    std::list<FreeTerm> free_terms_;
    V3fList const* scored_path_ = nullptr;
    NodeKeyMap nk_map_;
    bool align_before_export_ = true;

    /* accessors */
    virtual PathTeam* virtual_clone() const;
    virtual FreeTerm get_mutable_term() const;
    virtual NodeKey get_tip(bool const mutable_hint) const;
    virtual void mutation_invariance_check() const;
    virtual bool is_mutable(NodeKey const nk) const;
    virtual void postprocess_json(JSON& output) const;

    /* modifiers */
    virtual void reset();
    virtual void virtual_copy(NodeTeam const& other);
    void remove_free_terms(NodeKey const node);
    NodeKey add_node(ProtoModule const* const prot,
                     Transform const& tx = Transform(),
                     bool const innert = false,
                     size_t const n_ft_to_add = 2,
                     FreeTerm const* const exclude_ft = nullptr);
    NodeKey grow_tip(FreeTerm const& free_term_a,
                     ProtoLink const* pt_link = nullptr,
                     bool const innert = false);
    virtual void evaluate();
    virtual void calc_checksum();
    virtual void calc_score();
    virtual void penalize_collision();
    // For testing: builds node team from recipe and returns the starting node.
    virtual void virtual_implement_recipe(tests::Recipe const& recipe,
                                          FirstLastNodeKeyCallback const& postprocessor,
                                          Transform const& shift_tx);
public:
    /* ctors */
    PathTeam(WorkArea const* const wa,
             uint32_t const seed);
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
        virtual_implement_recipe(recipe, FirstLastNodeKeyCallback(), shift_tx);
        evaluate();
    }

    /* printers */
    virtual void print_to(std::ostream& os) const;
    virtual JSON to_json() const;

    /* tests */
    static TestStat test();
};  /* class PathTeam */

}  /* elfin */

#endif  /* end of include guard: PATH_TEAM_H_ */