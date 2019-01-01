#include "hinge_team.h"

#include "input_manager.h"
#include "path_generator.h"
#include "scoring.h"

namespace elfin {

/* private */
struct HingeTeam::PImpl {
    /* data */
    HingeTeam& _;

    /* ctors */
    PImpl(HingeTeam& interface) : _(interface) { }

    NodeKey place_hinge() {
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

        return _.add_free_node(proto_mod, ui_mod->tx);
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

FreeChain const& HingeTeam::pick_tip_chain() const {
    // If the only node is the hinge, then we allow modification to one random
    // tip.
    if (size() == 1) {
        return PathTeam::pick_tip_chain();
    }
    else {
        DEBUG_NOMSG(not hinge_);

        for (auto const& fc : free_chains_) {
            // There should be only one instance that this condition is true.
            if (fc.node != hinge_) {
                return fc;
            }
        }

        TRACE_NOMSG("No tip available");
        throw ExitException{1};
    }
}

void HingeTeam::mutation_invariance_check() const {
    // TRACE(free_chains_.size() != 2,
    //       "Invariance broken: %zu free chains\n",
    //       free_chains_.size());

    TRACE(size() == 0,
          "Invariance broken: size() = 0\n");
}

void HingeTeam::add_node_check(ProtoModule const* const prot) const {
    // Allow any module for hinge but other than that, same as PathTeam.
    if (hinge_) {
        PathTeam::add_node_check(prot);
    }
}

bool HingeTeam::can_delete_tip(NodeKey const tip) const {
    return tip != hinge_;
}

/* modifiers */
void HingeTeam::restart() {
    PathTeam::restart();

    hinge_ = nullptr;
    hinge_ = pimpl_->place_hinge();
}

void HingeTeam::virtual_copy(NodeTeam const& other) {
    try {  // Catch bad cast
        HingeTeam::operator=(static_cast<HingeTeam const&>(other));
    }
    catch (std::bad_cast const& e) {
        TRACE_NOMSG("Bad cast\n");
    }
}

void HingeTeam::calc_checksum() {
    // Unlike PathTeam, here we can compute one-way checksum from hinge.
    DEBUG_NOMSG(not hinge_);
    checksum_ = hinge_->gen_path().path_checksum();
}

void HingeTeam::calc_score() {
    auto const my_points =
        hinge_->gen_path().collect_points();

    // Should use simple_rms(), but there's some floating point rounding error
    // that's causing unit tests to fail. If we used inner_product() in
    // simple_rms(), that'd yield the satisfactory result but then an extra vector is
    // required. Until transform_reduce() is supported, Kabsch RMS should be
    // fine performance-wise?
    score_ = scoring::score(my_points,
                            work_area_->points);
    scored_tip_ = hinge_;
}

NodeKey HingeTeam::follow_recipe(tests::Recipe const& recipe,
                                 Transform const& shift_tx)
{
    hinge_ = nullptr;  // Clear hinge_ so add_node_check() can pass.

    hinge_ = PathTeam::follow_recipe(recipe, shift_tx);

    return hinge_;
}

/* public */
/* ctors */
HingeTeam::HingeTeam(WorkArea const* wa) :
    PathTeam(wa),
    pimpl_(make_pimpl())
{
    // Call place_hinge() after initializer list because hinge_ needs to be
    // initialiezd as nullptr.
    hinge_ = pimpl_->place_hinge();
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
    hinge_ = other.hinge_ ? nk_map_.at(other.hinge_) : nullptr;
    return *this;
}

HingeTeam& HingeTeam::operator=(HingeTeam && other) {
    PathTeam::operator=(std::move(other));
    std::swap(hinge_, other.hinge_);
    return *this;
}

}  /* elfin */