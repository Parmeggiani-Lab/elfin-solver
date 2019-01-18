#include "node_team.h"

#include <sstream>

#include "work_area.h"
#include "path_team.h"
#include "hinge_team.h"
#include "double_hinge_team.h"

namespace elfin {

/* protected */
/* modifiers */
void NodeTeam::reset() {
    checksum_ = 0x0000;
    score_ = INFINITY;
}

/* public */
/* ctors */
NodeTeam::NodeTeam(WorkArea const* const wa, uint32_t const seed) :
    work_area_(wa),
    seed_(seed)
{
    reset();
}

NodeTeamSP NodeTeam::create_team(WorkArea const* const work_area,
                                 uint32_t const seed) {
    TRACE_NOMSG(not work_area);

    NodeTeamSP team_up;
    switch (work_area->type) {
    case WorkType::FREE:
        team_up = std::make_unique<PathTeam>(work_area, seed);
        break;
    case WorkType::HINGE:
        team_up = std::make_unique<HingeTeam>(work_area, seed);
        break;
    case WorkType::LOOSE_HINGE:
        team_up = std::make_unique<HingeTeam>(work_area, seed, /*loose=*/true);
        break;
    case WorkType::DOUBLE_HINGE:
        team_up = std::make_unique<DoubleHingeTeam>(work_area, seed);
        break;
    default:
        throw BadWorkType(WorkTypeToCStr(work_area->type));
    }

    return team_up;
}

NodeTeam::NodeTeam(NodeTeam const& other) :
    NodeTeam(other.work_area_, other.seed_)
{ this->operator=(other); }

NodeTeam::NodeTeam(NodeTeam&& other) :
    NodeTeam(other.work_area_, other.seed_)
{ this->operator=(std::move(other)); }

/* modifiers */
NodeTeam& NodeTeam::operator=(NodeTeam const& other) {
    DEBUG_NOMSG(work_area_ != other.work_area_);
    checksum_ = other.checksum_;
    score_ = other.score_;
    return *this;
}

NodeTeam& NodeTeam::operator=(NodeTeam&& other) {
    DEBUG_NOMSG(work_area_ != other.work_area_);
    std::swap(checksum_, other.checksum_);
    std::swap(score_, other.score_);
    return *this;
}


}  /* elfin */