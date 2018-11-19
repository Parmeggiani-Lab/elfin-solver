#include "work_area.h"

#include <tuple>
#include <sstream>

#include "elfin_exception.h"

namespace elfin {

WorkArea::WorkArea(const JSON & j, const std::string & name) :
    name_(name) {
    size_t n_branches = 0;
    for (auto it = j.begin(); it != j.end(); ++it) {
        const JSON & jt_json = *it;

        auto j_itr = joints_.emplace(it.key(), UIJoint(jt_json, it.key()));
        const size_t n_nbs = jt_json["neighbours"].size();
        n_branches += n_nbs > 2;

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

        if (n_nbs == 1) {
            leaf_joints_.push_back(ptr);
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

V3fList WorkArea::to_V3fList() const {
    V3fList res;
    if (leaf_joints_.size() != 2)
        die("Size of leaf_joints_ not exactly 2 in work_area: %s\n", name_.c_str());

    UIJoint const * prev = nullptr, * tmp = nullptr;
    UIJoint const * j = leaf_joints_.at(0);
    while (1) {
        res.emplace_back(j->tran());

        if (j == leaf_joints_.at(1))
            break;

        tmp = j;
        std::string next_name = j->neighbours().at(0);
        if (prev and next_name == prev->name())
            next_name = j->neighbours().at(1);
        j = &joints_.at(next_name);
        prev = j;
    }

    return res;
}

}  /* elfin */