#include "hinge_team.h"

namespace elfin {

/* private */
struct HingeTeam::PImpl {
    /* data */
    HingeTeam& _;

    /* ctors */
    PImpl(HingeTeam& interface) : _(interface) {
        place_hinges();
    }

    void place_hinges() {
        auto const& occ_joints = _.work_area_->occupied_joints;
        DEBUG_NOMSG(occ_joints.size() == 0);
        DEBUG_NOMSG(occ_joints.size() > 2);

        if (occ_joints.size() == 1) {
            for (auto & oj : occ_joints) {
                JUtil.warn("oj: %s\n", oj->to_string().c_str());
            }
            for (auto & lj : _.work_area_->leaf_joints) {
                JUtil.warn("lj: %s\n", lj->to_string().c_str());
            }
        }
    }
};

/* modifiers */
std::unique_ptr<HingeTeam::PImpl> HingeTeam::make_pimpl() {
    return std::make_unique<PImpl>(*this);
}

/* protected */
/* accessors */
HingeTeam* HingeTeam::virtual_clone() const {
    return new HingeTeam(*this);
}

/* modifiers */
void HingeTeam::virtual_copy(NodeTeam const& other) {
    try {  // Catch bad cast
        HingeTeam::operator=(static_cast<HingeTeam const&>(other));
    }
    catch (std::bad_cast const& e) {
        TRACE_NOMSG("Bad cast\n");
    }
}

void HingeTeam::calc_score() {
    // UNIMP();
    // Need to take into account the fact that the joint that superimposes the
    // hinge module is not scored.
}

/* public */
/* ctors */
HingeTeam::HingeTeam(WorkArea const* wa) :
    PathTeam(wa),
    pimpl_(make_pimpl()) {}

// Recipe ctor is used for testing.
HingeTeam::HingeTeam(WorkArea const* wa, tests::Recipe const& recipe) :
    PathTeam(wa, recipe),
    pimpl_(make_pimpl())
{
}

HingeTeam::HingeTeam(HingeTeam const& other) :
    HingeTeam(other.work_area_)
{
    HingeTeam::operator=(other);
}

HingeTeam::HingeTeam(HingeTeam&& other)  :
    HingeTeam(other.work_area_)
{
    HingeTeam::operator=(std::move(other));
}

/* dtors */
HingeTeam::~HingeTeam() {}

/* modifiers */
HingeTeam& HingeTeam::operator=(HingeTeam const& other) {
    PathTeam::operator=(other);
    return *this;
}

HingeTeam& HingeTeam::operator=(HingeTeam && other) {
    PathTeam::operator=(std::move(other));
    return *this;
}

}  /* elfin */