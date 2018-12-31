#include "hinge_team.h"

#include "input_manager.h"
#include "path_generator.h"

namespace elfin {

/* private */
struct HingeTeam::PImpl {
    /* data */
    HingeTeam& _;

    /* ctors */
    PImpl(HingeTeam& interface) : _(interface) { }

    NodeKey find_hinge() {
        auto const& occ_joints = _.work_area_->occupied_joints;
        size_t const n_occ_joints = occ_joints.size();
        DEBUG_NOMSG(n_occ_joints != 1);

        auto& occ_joint = *begin(occ_joints);

        // Place hinge node.
        auto const& ui_mod = occ_joint->occupant.ui_module;
        auto proto_mod = XDB.get_module(ui_mod->module_name);

        // Here, if elfin-ui provides info about which specific chain of
        // the hinge module interfaces with the next unknown module
        // (joint), then we could remove free chains that are outside of
        // this work area. However right now that functionality is not
        // implemented by elfin-ui so we'll leave the selection to GA.
        //
        //
        // auto const& neighbors = occ_joint->neighbors;
        // DEBUG_NOMSG(neighbors.size() != 1);
        // auto nb_ui_joint = *begin(neighbors);
        // auto proto_chain_itr = std::find_if(...);

        return _.add_member(proto_mod, ui_mod->tx);
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

void HingeTeam::calc_checksum() {
    //
    // Unlike PathTeam, here we can compute one-way checksum from hinge.
    //
    DEBUG_NOMSG(size() == 0);

    checksum_ = PathGenerator::path_checksum(hinge_);
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
    pimpl_(make_pimpl()),
    hinge_(pimpl_->find_hinge())
{
    DEBUG_NOMSG(not hinge_);
}

// Recipe ctor is used for testing.
HingeTeam::HingeTeam(WorkArea const* wa, tests::Recipe const& recipe) :
    PathTeam(wa, recipe),
    pimpl_(make_pimpl()),
    hinge_(pimpl_->find_hinge())
{
    DEBUG_NOMSG(not hinge_);
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
    hinge_ = other.hinge_;
    return *this;
}

HingeTeam& HingeTeam::operator=(HingeTeam && other) {
    PathTeam::operator=(std::move(other));
    std::swap(hinge_, other.hinge_);
    return *this;
}

}  /* elfin */