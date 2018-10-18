#include "work_area.h"

namespace elfin {

std::shared_ptr<WorkArea> WorkArea::from_json(const JSON & pgn, const JSON & networks, const std::string & name) {
    std::shared_ptr<WorkArea> wa = std::make_shared<WorkArea>(name);

    size_t n_hinges = 0, n_branches = 0;
    for (auto it = pgn.begin(); it != pgn.end(); ++it) {
        const JSON & joint = *it;
        std::shared_ptr<Point3f> p = Point3f::from_json(joint["trans"]);
        wa->push_back(*p);

        n_hinges += joint["occupant"] != "";
        n_branches += joint["neighbours"].size() > 2;
    }

    if (n_branches == 0)
    {
        switch (n_hinges) {
        case 0: {
            wa->type_ = FREE;
            break;
        }
        case 1: {
            wa->type_ = ONE_HINGE;
            break;
        }
        case 2: {
            wa->type_ = TWO_HINGE;
            break;
        }
        default:
        {
            wa->type_ = COMPLEX;
        }
        }
    } else {
        wa->type_ = COMPLEX;
    }

    // TODO: should we keep the n_hinges/n_branches or the joints themselves in wa?

    return wa;
}

}  /* elfin */