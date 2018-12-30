#ifndef NODE_TEAM_H_
#define NODE_TEAM_H_

#include <memory>
#include <unordered_set>

#include "work_area.h"
#include "checksum.h"
#include "mutation.h"

namespace elfin {


/* Fwd Decl */
class NodeTeam;
typedef std::unique_ptr<NodeTeam> NodeTeamSP;


class NodeTeam : public Printable {
protected:
    /* data */
    WorkArea const* work_area_ = nullptr;
    Crc32 checksum_ = 0x0000;
    float score_ = INFINITY;

    /* accessors */
    // Allow copying from/cloning of derived objects without cast.
    virtual void virtual_copy(NodeTeam const& other) = 0;
    virtual NodeTeam* virtual_clone() const = 0;

public:
    /* ctors */
    NodeTeam(WorkArea const* const wa) : work_area_(wa) {}
    static NodeTeamSP create_team(WorkArea const* const work_area);
    void copy(NodeTeam const& other) { virtual_copy(other); }
    NodeTeamSP clone() const { return NodeTeamSP(virtual_clone()); }

    /* dtors */
    virtual ~NodeTeam() {}

    /* accessors */
    float score() const { return score_; }
    Crc32 checksum() const { return checksum_; }
    virtual size_t size() const = 0;
    static bool ScoreCompareSP(
        NodeTeamSP const& lhs,
        NodeTeamSP const& rhs) { return lhs->score_ < rhs->score_; }

    /* modifiers */
    virtual void randomize() = 0;
    virtual mutation::Mode evolve(
        NodeTeam const& mother,
        NodeTeam const& father) = 0;

    /* printers */
    virtual JSON to_json() const = 0;
};  /* class NodeTeam */

}  /* elfin */

#endif  /* end of include guard: NODE_TEAM_H_ */