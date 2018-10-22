#include "work_area.h"

#include <tuple>

namespace elfin {

WorkArea::WorkArea(const JSON & j, const std::string & name) :
    name_(name) {
    size_t n_branches = 0;
    for (auto it = j.begin(); it != j.end(); ++it) {
        const JSON & jt_json = *it;

        auto j_itr = joints_.emplace(it.key(), UIJoint(jt_json, it.key()));
        n_branches += jt_json["neighbours"].size() > 2;

        auto & key_val = *j_itr.first;
        UIJoint * ptr = &(key_val.second);
        if (jt_json["occupant"] != "") {
            ptr->occupant_triple_ =
                std::make_tuple(
                    jt_json["occupant_parent"],
                    jt_json["occupant"],
                    nullptr);
            occupied_joints_.push_back(ptr);
        }

        if (jt_json["hinge"] != "") {
            ptr->hinge_tuple_ = std::make_tuple(jt_json["hinge"], nullptr);
            hinged_joints_.push_back(ptr);
        }
    }

    if (n_branches == 0) {
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
    for (auto & itr : joints_) {
        res.emplace_back(itr.second.tran());
    }
    return res;
}

}  /* elfin */