#ifndef NODE_TEAM_H_
#define NODE_TEAM_H_

#include <memory>

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

class NodeTeam;
typedef std::shared_ptr<NodeTeam> NodeTeamSP;

class NodeTeam {
protected:
    /* types */
    typedef VectorMap<Node *> Nodes;

    /* data */
    WorkArea const& work_area_;
    Nodes nodes_;
    FreeChainList free_chains_;
    Crc32 checksum_ = 0x0000;
    float score_ = INFINITY;

    /* accessors */
    bool collides(
        Vector3f const& new_com,
        float const mod_radius) const;
    void check_work_area(NodeTeam const& other) const;
    virtual NodeTeam * clone_impl() const = 0;

    /* modifiers */
    void disperse();
    Node* add_member(
        ProtoModule const* prot,
        Transform const& tx = Transform());
    void remove_member(Node* node);
    void remove_free_chains(Node* node);

public:
    /* ctors */
    NodeTeam(WorkArea const& work_area);
    NodeTeam(NodeTeam const& other);
    NodeTeam(NodeTeam && other);

    /* dtors */
    virtual ~NodeTeam();

    /* accessors */
    NodeTeamSP clone() const;
    Nodes const& nodes() const { return nodes_; }
    FreeChainList const& free_chains() const { return free_chains_; }
    size_t size() const { return nodes_.size(); }
    float score() const { return score_; }
    Crc32 checksum() const { return checksum_; }
    static bool ScoreCompareSP(
        NodeTeamSP const& lhs,
        NodeTeamSP const& rhs) {
        return lhs->score_ < rhs->score_;
    }

    /* modifiers */
    NodeTeam& operator=(NodeTeam const& other);
    NodeTeam& operator=(NodeTeam && other);

    virtual MutationMode mutate_and_score(
        NodeTeam const& mother,
        NodeTeam const& father) = 0;
    virtual void randomize() = 0;

    /* printers */
    virtual std::string to_string() const = 0;
    virtual StrList get_node_names() const = 0;

};  /* class NodeTeam*/

}  /* elfin */

#endif  /* end of include guard: NODE_TEAM_H_ */