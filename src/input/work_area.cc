#include "work_area.h"

namespace elfin {

WorkArea::WorkArea(const JSON & j, const std::string & name) :
    name_(name) {

    size_t n_branches = 0;
    for (auto it = j.begin(); it != j.end(); ++it) {
        const JSON & joint = *it;

        joints_.emplace_back(joint, it.key());
        n_branches += joint["neighbours"].size() > 2;

        if (joint["occupant"] != "") {
            // occupied_joints_.push_back(&joints_.back());
            /*
            Use map!!!
            */
        }
    }

    if (n_branches == 0)
    {
        switch (occupied_joints_.size()) {
        case 0: {
            type_ = FREE;
            break;
        }
        case 1: {
            type_ = ONE_HINGE;
            break;
        }
        case 2: {
            type_ = TWO_HINGE;
            break;
        }
        default:
        {
            type_ = COMPLEX;
        }
        }
    } else {
        type_ = COMPLEX;
    }
}

Points3f WorkArea::to_points3f() const {
    Points3f res;
    for (auto & j : joints_) {
        res.emplace_back(j.tran());
    }
    return res;
}

}  /* elfin */