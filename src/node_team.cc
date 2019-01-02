#include "node_team.h"

#include <sstream>

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
NodeTeamSP NodeTeam::create_team(WorkArea const* const work_area) {
    TRACE_NOMSG(not work_area);

    NodeTeamSP team_up;
    switch (work_area->type) {
    case WorkType::FREE:
        team_up = std::make_unique<PathTeam>(work_area);
        break;
    case WorkType::HINGED:
        team_up = std::make_unique<HingeTeam>(work_area);
        break;
    case WorkType::DOUBLE_HINGED:
        team_up = std::make_unique<DoubleHingeTeam>(work_area);
        break;
    default:
        bad_work_type(work_area->type);
        return nullptr;  // Suppress warning.
    }

    return team_up;
}

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