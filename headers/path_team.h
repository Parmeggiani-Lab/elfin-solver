#ifndef PATH_TEAM_H_
#define PATH_TEAM_H_

#include "node_team.h"
#include "node.h"
#include "test_data.h"

namespace elfin {

class PathTeam : public NodeTeam {
private:
    /* type */
    class PImpl;

    /* data */
    std::unique_ptr<PImpl> p_impl_;

    /* accessors */
    //
    // In a PathTeam there are either 0 or 2 tips at any given time. The
    // nodes network is thus a simple path. We walk the path to collect the 3D
    // points in order.
    //
    V3fList collect_points(NodeKey tip_node) const;

    /*modifiers */
    std::unique_ptr<PImpl> init_pimpl();
    NodeKey grow_tip(
        FreeChain const free_chain_a,
        ProtoLink const* ptlink = nullptr);
protected:
    /* data */
    std::unordered_map<NodeKey, NodeSP> nodes_;
    FreeChainList free_chains_;

    /* accessors */
    virtual void virtual_copy(NodeTeam const& other);
    virtual PathTeam* virtual_clone() const;

    /* modifiers */
    NodeKey add_member(
        ProtoModule const* prot,
        Transform const& tx = Transform());
    void remove_free_chains(NodeKey const node);
public:
    /* ctors */
    PathTeam(WorkArea const* wa);
    // Recipe ctor is used for testing.
    PathTeam(WorkArea const* wa, tests::Recipe const& recipe);
    PathTeam(PathTeam const& other);
    PathTeam(PathTeam&& other);

    /* dtors */
    virtual ~PathTeam();

    /* accessors */
    virtual size_t size() const { return nodes_.size(); }
    FreeChainList const& free_chains() const { return free_chains_; }

    /* modifiers */
    PathTeam& operator=(PathTeam const& other);
    PathTeam& operator=(PathTeam && other);
    virtual void randomize();
    virtual mutation::Mode evolve(
        NodeTeam const& mother,
        NodeTeam const& father);

    /* printers */
    virtual void print_to(std::ostream& os) const;
    virtual JSON to_json() const;

    /* tests */
    static TestStat test();
};  /* class PathTeam */

}  /* elfin */

#endif  /* end of include guard: PATH_TEAM_H_ */