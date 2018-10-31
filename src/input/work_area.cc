#include "work_area.h"

#include <tuple>
#include <sstream>

#include "elfin_exception.h"
#include "free_candidate.h"
#include "one_hinge_candidate.h"
#include "two_hinge_candidate.h"

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

Candidate * WorkArea::new_candidate(bool randomize) const {
    Candidate * c = nullptr;

    err("Incomplete implementation: new_candidate()\n");
    err("Handle randomize according to WorkArea\n");
    try {
        switch (type_) {
        case FREE:
            c = new FreeCandidate();
            break;
        case ONE_HINGE:
            c = new OneHingeCandidate();
            break;
        case TWO_HINGE:
            c = new TwoHingeCandidate();
            break;
        default:
            std::stringstream ss;
            ss << "Unimplemented WorkArea type: ";
            ss << WorkTypeNames[type_] << std::endl;
            throw ElfinException(ss.str());
        }
    }
    catch (std::exception const& exc) {
        err("Exception caught: \n");
        raw_at(LOG_ERROR, exc.what());
        throw exc;
    }

    return c;
}

}  /* elfin */