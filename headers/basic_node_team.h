#ifndef BASIC_NODE_TEAM_H_
#define BASIC_NODE_TEAM_H_

#include "node_team.h"

namespace elfin {

struct TestStat;

namespace tests {

class Step;
typedef std::vector<Step> StepList;

}  /* tests */

class BasicNodeTeam : public NodeTeam {
private:
    /* data */
    class PImpl;
    std::unique_ptr<PImpl> p_impl_;

    /* accessors */
    Crc32 calc_checksum() const;
    float calc_score() const;
    //
    // In a BasicNodeTeam there are either 0 or 2 tips at any given time. The
    // nodes network is thus a simple path. We walk the path to collect the 3D
    // points in order.
    //
    V3fList collect_points(NodeSP const& tip_node) const;

    /*modifiers */
    std::unique_ptr<PImpl> init_pimpl();
    NodeSP grow_tip(
        FreeChain const free_chain_a,
        ProtoLink const* ptlink = nullptr);
protected:
    /* types */
    typedef std::unordered_set<NodeSP> Nodes;

    /* data */
    Nodes nodes_;
    FreeChainList free_chains_;

    /* accessors */
    virtual BasicNodeTeam* virtual_clone() const;

    /* modifiers */
    NodeSP add_member(
        ProtoModule const* prot,
        Transform const& tx = Transform());
    void remove_free_chains(NodeSP const& node);
public:
    /* ctors */
    using NodeTeam::NodeTeam;
    BasicNodeTeam(WorkArea const* wa);
    BasicNodeTeam(BasicNodeTeam const& other);
    BasicNodeTeam(BasicNodeTeam&& other);
    virtual void copy_from(NodeTeam const& other);

    /* dtors */
    virtual ~BasicNodeTeam();

    /* accessors */
    virtual size_t size() const { return nodes_.size(); }
    FreeChainList const& free_chains() const { return free_chains_; }

    /* modifiers */
    BasicNodeTeam& operator=(BasicNodeTeam const& other);
    BasicNodeTeam& operator=(BasicNodeTeam && other);
    virtual void randomize();
    virtual mutation::Mode evolve(
        NodeTeam const& mother,
        NodeTeam const& father);

    /* printers */
    virtual void print_to(std::ostream& os) const;
    virtual JSON to_json() const;

    /* tests */
    static BasicNodeTeam build_team(tests::StepList const& steps);
    static TestStat test();
};  /* class BasicNodeTeam */

}  /* elfin */

#endif  /* end of include guard: BASIC_NODE_TEAM_H_ */