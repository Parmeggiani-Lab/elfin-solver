#ifndef BASIC_NODE_TEAM_H_
#define BASIC_NODE_TEAM_H_

#include "node_team.h"

namespace elfin {

struct TestStat;

class BasicNodeTeam : public NodeTeam {
private:
    /* data */
    class PImpl;
    std::unique_ptr<PImpl> p_impl_;

    /* accessors */
    Crc32 calc_checksum() const;
    float calc_score() const;
    V3fList collect_points(NodeSP const& tip_node) const;

    /*modifiers */
    std::unique_ptr<PImpl> init_pimpl();
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
    virtual MutationMode mutate_and_score(
        NodeTeam const& mother,
        NodeTeam const& father);
    virtual void randomize();

    /* printers */
    virtual std::string to_string() const;
    virtual StrList get_node_names() const;

    /* tests */
    static TestStat test();
};  /* class BasicNodeTeam */

}  /* elfin */

#endif  /* end of include guard: BASIC_NODE_TEAM_H_ */