#include "node_team.h"

#include <sstream>

#include "path_team.h"
#include "hinge_team.h"

namespace elfin {

/* public */
/* ctors */
NodeTeamSP NodeTeam::create_team(WorkArea const* const work_area) {
    NodeTeamSP team_up;

    switch (work_area->type) {
    case WorkType::FREE:
        team_up = std::make_unique<PathTeam>(work_area);
        break;
    case WorkType::ONE_HINGE:
    case WorkType::TWO_HINGE:
        team_up = std::make_unique<HingeTeam>(work_area);
        break;
    default:
        bad_work_type(work_area->type);
        return nullptr; // Suppress warning.
    }

    return team_up;
}

}  /* elfin */