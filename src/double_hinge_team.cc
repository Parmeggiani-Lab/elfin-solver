#include "double_hinge_team.h"

#include "input_manager.h"

namespace elfin {

/* private */
struct DoubleHingeTeam::PImpl {
    /* data */
    DoubleHingeTeam& _;

    /* ctors */
    PImpl(DoubleHingeTeam& interface) : _(interface) {}

    NodeKey get_hinge() {
        auto const& occ_joints = _.work_area_->occupied_joints;
        size_t const n_occ_joints = occ_joints.size();
        DEBUG_NOMSG(n_occ_joints != 2);

        auto& occ_joint = random::pick(occ_joints);

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

/*modifiers */
std::unique_ptr<DoubleHingeTeam::PImpl> DoubleHingeTeam::make_pimpl() {
    return std::make_unique<PImpl>(*this);
}

/* protected */
/* accessors */
DoubleHingeTeam* DoubleHingeTeam::virtual_clone() const {
    return new DoubleHingeTeam(*this);
}

/* modifiers */
void DoubleHingeTeam::restart() {
    PathTeam::restart();

    hinge_ = nullptr;
    hinge_ = pimpl_->get_hinge();
}

void DoubleHingeTeam::virtual_copy(NodeTeam const& other) {
    try {  // Catch bad cast
        HingeTeam::operator=(
            static_cast<DoubleHingeTeam const&>(other));
    }
    catch (std::bad_cast const& e) {
        TRACE_NOMSG("Bad cast\n");
    }
}

/*public*/
/* ctors */
DoubleHingeTeam::DoubleHingeTeam(WorkArea const* wa) :
    HingeTeam(wa),
    pimpl_(make_pimpl()) {}

DoubleHingeTeam::DoubleHingeTeam(DoubleHingeTeam const& other) :
    DoubleHingeTeam(other.work_area_) {
    HingeTeam::operator=(other);

}
DoubleHingeTeam::DoubleHingeTeam(DoubleHingeTeam&& other) :
    DoubleHingeTeam(other.work_area_) {
    HingeTeam::operator=(std::move(other));
}

/* dtors */
DoubleHingeTeam::~DoubleHingeTeam() {}

}  /* elfin */