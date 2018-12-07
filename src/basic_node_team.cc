#include "basic_node_team.h"

#include "jutil.h"
#include "basic_node_generator.h"
#include "kabsch.h"

namespace elfin {

BasicNodeTeam::BasicNodeTeam(
    const BasicNodeTeam & other) : NodeTeam(other) {
}

BasicNodeTeam::BasicNodeTeam(
    BasicNodeTeam && other) : NodeTeam(other) {
}

BasicNodeTeam * BasicNodeTeam::clone() const {
    return new BasicNodeTeam(*this);
}

float BasicNodeTeam::score(const WorkArea & wa) const {
    /*
     * In a BasicNodeTeam there are 2 and only 2 tips at all times. The nodes
     * network is thus a simple path. We walk the path to collect the 3D
     * points in order.
     */

    Node * rand_tip = random_free_seeker(TerminusType::ANY).node;
    BasicNodeGenerator<Node> bng = BasicNodeGenerator<Node>(rand_tip);

    V3fList points;
    while (not bng.is_done()) {
        points.emplace_back(bng.next()->tx().collapsed());
    }

    DEBUG(points.size() != size(),
          string_format("points.size()=%lu, size()=%lu\n",
                        points.size(), this->size()));

    return kabsch_score(points, wa);
}

StrList BasicNodeTeam::get_node_names() const {
    StrList res;

    Node * rand_tip = random_free_seeker(TerminusType::ANY).node;
    BasicNodeGenerator<Node> bng = BasicNodeGenerator<Node>(rand_tip);

    while (not bng.is_done()) {
        res.emplace_back(bng.next()->prototype()->name);
    }

    DEBUG(res.size() != size(),
          string_format("res.size()=%lu, size()=%lu\n",
                        res.size(), this->size()));

    return res;
}

}  /* elfin */