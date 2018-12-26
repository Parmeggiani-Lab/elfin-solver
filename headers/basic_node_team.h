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
    /* accessors */
    virtual BasicNodeTeam* clone_impl() const;

public:
    /* ctors */
    using NodeTeam::NodeTeam;
    BasicNodeTeam(WorkArea const* wa);
    BasicNodeTeam(BasicNodeTeam const& other);
    BasicNodeTeam(BasicNodeTeam&& other);

    /* dtors */
    virtual ~BasicNodeTeam();

    /* modifiers */
    virtual mutation::Mode mutate_and_score(
        NodeTeam const& mother,
        NodeTeam const& father);
    virtual void randomize();

    /* printers */
    virtual std::string to_string() const;
    virtual JSON gen_nodes_json() const;

    /* tests */
    static BasicNodeTeam build_team(tests::StepList const& steps);
    static TestStat test();
};  /* class BasicNodeTeam */

}  /* elfin */

#endif  /* end of include guard: BASIC_NODE_TEAM_H_ */