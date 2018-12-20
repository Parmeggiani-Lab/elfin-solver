#ifndef BASIC_NODE_TEAM_H_
#define BASIC_NODE_TEAM_H_

#include "node_team.h"
#include "node_team_recipe.h"
#include "test_stat.h"

namespace elfin {

class BasicNodeTeam : public NodeTeam {
private:
    /* types */
    struct DeletePoint;
    struct InsertPoint;
    struct SwapPoint;
    struct CrossPoint;

    /* accessors */
    virtual BasicNodeTeam* clone_impl() const;
    Crc32 calc_checksum() const;
    float calc_score() const;

    /*modifiers */
    void fix_limb_transforms(Link const& arrow);
    NodeSP grow_tip(
        FreeChain const free_chain_a,
        ProtoLink const* ptlink = nullptr);
    void nip_tip(NodeSP const& tip_node);
    void build_bridge(
        InsertPoint const& insert_point,
        FreeChain::Bridge const* bridge = nullptr);
    void sever_limb(Link const& arrow);
    void copy_limb(Link const& m_arrow, Link const& f_arrow);

    bool erode_mutate();
    bool delete_mutate();
    bool insert_mutate();
    bool swap_mutate();
    bool cross_mutate(
        NodeTeam const& father);
    bool regenerate();
    bool randomize_mutate();

public:
    /* ctors */
    using NodeTeam::NodeTeam;
    // A ctor used to artificially create test teams
    BasicNodeTeam(NodeTeamRecipe const& recipe);

    /* dtors */
    virtual ~BasicNodeTeam() {}

    /* modifiers */
    virtual MutationMode mutate_and_score(
        NodeTeam const& mother,
        NodeTeam const& father);
    virtual void randomize() { randomize_mutate(); }

    /* printers */
    virtual std::string to_string() const;
    virtual StrList get_node_names() const;

    /* tests */
    static TestStat test();
};  /* class BasicNodeTeam */

}  /* elfin */

#endif  /* end of include guard: BASIC_NODE_TEAM_H_ */