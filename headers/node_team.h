#ifndef NODE_TEAM_H_
#define NODE_TEAM_H_

#include <memory>
#include <unordered_set>

#include "work_area.h"
#include "checksum.h"
#include "mutation.h"

namespace elfin {

class NodeTeam;

typedef std::unique_ptr<NodeTeam> NodeTeamSP;

class NodeTeam : public Printable {
protected:
    /* data */
    WorkArea const* work_area_ = nullptr;
    Crc32 checksum_ = 0x0000;
    float score_ = INFINITY;

    /* accessors */
    // bool collides(
    //     Vector3f const& new_com,
    //     float const mod_radius) const;
    virtual NodeTeam * virtual_clone() const = 0;

public:
    /* ctors */
    static NodeTeamSP create_team(WorkArea const* work_area);
    NodeTeamSP clone() const;
    virtual void copy_from(NodeTeam const& other) = 0;

    /* dtors */
    virtual ~NodeTeam() {}

    /* accessors */
    float const& score = score_;
    Crc32 const& checksum = checksum_;
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

    /* tests */
};  /* class NodeTeam */

}  /* elfin */

#endif  /* end of include guard: NODE_TEAM_H_ */