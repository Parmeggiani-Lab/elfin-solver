#include "node_team.h"

#include <sstream>

#include "path_team.h"

#define FREE_CHAIN_VM_RESERVE_SIZE 16

namespace elfin {

/* protected */
/* accessors */
// bool NodeTeam::collides(
//     Vector3f const& new_com,
//     float const mod_radius) const {

//     for (auto& node : nodes_) {
//         float const sq_com_dist = node->tx_.collapsed().sq_dist_to(new_com);
//         float const required_com_dist =
//             mod_radius + node->prototype_->radius;
//         if (sq_com_dist < (required_com_dist * required_com_dist)) {
//             return true;
//         }
//     }

//     return false;
// }

/* public */
/* ctors */
NodeTeamSP NodeTeam::create_team(WorkArea const* work_area) {
    NodeTeamSP team_up;

    switch (work_area->type()) {
    case WorkType::FREE:
        team_up = std::make_unique<PathTeam>(work_area);
        // case WorkType::ONE_HINGE:
        //     return std::make_unique<OneHingeNodeTeam>(work_area);
        // case WorkType::TWO_HINGE:
        //     return std::make_unique<TwoHingeNodeTeam>(work_area);
        break;
    default:
        bad_work_type(work_area->type());
        return nullptr; // Suppress warning.
    }

    return team_up;
}

NodeTeamSP NodeTeam::clone() const {
    return NodeTeamSP(virtual_clone());
}

}  /* elfin */