#ifndef BASIC_NODE_TEAM_H_
#define BASIC_NODE_TEAM_H_

#include "node_team.h"

namespace elfin {

class BasicNodeTeam : public NodeTeam {
private:
    /* types */
    struct DeletePoint;
    struct InsertPoint;
    struct SwapPoint;
    struct CrossPoint;

    /* accessors */
    Crc32 calc_checksum() const;
    float calc_score(WorkArea const* wa) const;

    /*modifiers */
    void fix_limb_transforms(Link const& arrow);
    void grow_tip(FreeChain const free_chain_a);
    Node* nip_tip(Node* tip_node);
    void build_bridge(
        InsertPoint const& insert_point,
        FreeChain::Bridge const* bridge = nullptr);

    bool erode_mutate();
    bool delete_mutate();
    bool insert_mutate();
    bool swap_mutate();
    bool cross_mutate(
        NodeTeam const* father);
    bool regenerate();
    bool randomize_mutate();

public:
    /* ctors */
    using NodeTeam::NodeTeam;
    BasicNodeTeam() : NodeTeam() {}
    BasicNodeTeam(BasicNodeTeam const& other);
    virtual BasicNodeTeam* clone() const;

    /* dtors */
    virtual ~BasicNodeTeam() {}

    /* modifiers */
    virtual void deep_copy_from(NodeTeam const* other);
    virtual MutationMode mutate_and_score(
        NodeTeam const* mother,
        NodeTeam const* father,
        WorkArea const* wa);
    virtual void randomize() { randomize_mutate(); }

    /* printers */
    virtual std::string to_string() const;
    virtual StrList get_node_names() const;

};  /* class BasicNodeTeam*/

}  /* elfin */

#endif  /* end of include guard: BASIC_NODE_TEAM_H_ */