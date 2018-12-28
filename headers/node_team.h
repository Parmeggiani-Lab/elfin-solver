#ifndef NODE_TEAM_H_
#define NODE_TEAM_H_

#include <memory>
#include <unordered_set>

#include "node.h"
#include "work_area.h"
#include "checksum.h"
#include "mutation.h"

namespace elfin {

class NodeTeam;
typedef std::shared_ptr<NodeTeam> NodeTeamSP;

class NodeTeam : public Printable {
protected:
    /* types */
    typedef std::unordered_set<NodeSP> NodeSPVMap;

    /* data */
    WorkArea const* work_area_ = nullptr;
    NodeSPVMap nodes_;
    FreeChainList free_chains_;
    Crc32 checksum_ = 0x0000;
    float score_ = INFINITY;

    /* accessors */
    // bool collides(
    //     Vector3f const& new_com,
    //     float const mod_radius) const;
    virtual NodeTeam * clone_impl() const = 0;

    /* modifiers */
    void reset();
    NodeSP add_member(
        ProtoModule const* prot,
        Transform const& tx = Transform());
    void remove_free_chains(NodeSP const& node);

public:
    /* ctors */
    NodeTeam(WorkArea const* work_area);
    NodeTeam(NodeTeam const& other);
    NodeTeam(NodeTeam&& other);

    /* dtors */
    virtual ~NodeTeam();

    /* accessors */
    NodeTeamSP clone() const;
    NodeSPVMap const& nodes() const { return nodes_; }
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

    virtual mutation::Mode mutate_and_score(
        NodeTeam const& mother,
        NodeTeam const& father) = 0;
    virtual void randomize() = 0;

    /* printers */
    virtual JSON gen_nodes_json() const = 0;

    /* tests */
};  /* class NodeTeam */

}  /* elfin */

#endif  /* end of include guard: NODE_TEAM_H_ */