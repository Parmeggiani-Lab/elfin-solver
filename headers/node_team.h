#ifndef NODE_TEAM_H_
#define NODE_TEAM_H_

#include <deque>

#include "node.h"
#include "work_area.h"
#include "vector_map.h"
#include "checksum.h"
#include "mutation_modes.h"

namespace elfin {

#define FOREACH_PMM(MACRO) \
    MACRO(SWAP_MODE) \
    MACRO(INSERT_MODE) \
    MACRO(DELETE_MODE) \
    MACRO(ENUM_SIZE) \

GEN_ENUM_AND_STRING(PointMutateMode, PointMutateModeNames, FOREACH_PMM);

class NodeTeam {
protected:
    /* types */
    typedef VectorMap<Node *> Nodes;

    /* data */
    Nodes nodes_;
    FreeChainList free_chains_;

    /* ctors */

    /* accessors */
    bool collides(
        Vector3f const & new_com,
        float const mod_radius) const;

    /* modifiers */
    void disperse();
    Node * add_member(
        ProtoModule const* prot,
        Transform const& tx = Transform());
    void remove_member(Node *  node);
    void remove_member_chains(Node *  node);

public:
    /* ctors */
    NodeTeam();
    NodeTeam(NodeTeam && other);
    NodeTeam(NodeTeam const& other) = delete;
    virtual NodeTeam * clone() const = 0;

    /* dtors */
    virtual ~NodeTeam();

    /* accessors */
    Nodes const& nodes() const { return nodes_; }
    FreeChainList const& free_chains() const { return free_chains_; }
    size_t size() const { return nodes_.size(); }

    virtual float score(WorkArea const* wa) const = 0;
    virtual Crc32 checksum() const = 0;

    /* modifiers */
    NodeTeam & operator=(NodeTeam const& other) = delete;
    NodeTeam & operator=(NodeTeam && other);

    virtual void deep_copy_from(NodeTeam const* other) = 0;
    virtual MutationMode mutate(
        NodeTeam const* mother,
        NodeTeam const* father) = 0;
    virtual void randomize() = 0;

    /* printers */
    virtual std::string to_string() const = 0;
    virtual StrList get_node_names() const = 0;

};  /* class NodeTeam*/

}  /* elfin */

#endif  /* end of include guard: NODE_TEAM_H_ */