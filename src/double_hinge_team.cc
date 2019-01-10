#include "double_hinge_team.h"

#include "input_manager.h"
#include "path_generator.h"

namespace elfin {

/* private */
struct DoubleHingeTeam::PImpl {
    /* data */
    DoubleHingeTeam& _;

    /* ctors */
    PImpl(DoubleHingeTeam& interface) : _(interface) {}

    void find_hinge2() {
        DEBUG_NOMSG(not _.hinge_ui_joint_);
        DEBUG_NOMSG(not _.hinge_);

        auto const& omap = _.work_area_->occupants;

        auto const itr = find_if(begin(omap), end(omap),
        [&](auto const & omap_pair) {
            return omap_pair.second != _.hinge_ui_joint_;
        });
        DEBUG_NOMSG(itr == end(omap));

        _.hinge_ui_joint2_ = itr->second;
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

void DoubleHingeTeam::postprocess_json(JSON& output) const {
    HingeTeam::postprocess_json(output);  // Remove first hinge.

    if (output.size() > 0) {
        // Skip last *supposed* hinge.
        output.erase(output.size() - 1);
    }
}

/* modifiers */
void DoubleHingeTeam::reset() {
    HingeTeam::reset();
    hinge_ui_joint2_ = nullptr;

    pimpl_->find_hinge2();
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
void DoubleHingeTeam::evavluate() {
    // Run djistrak to complete the mutable end if team does not end in second
    // hinge.
    auto const mutable_tip = get_tip(/*mutable_hint=*/true);
    std::string const& mutable_tip_mod_name =
        mutable_tip->prototype_->name;
    std::string const& hinge2_mod_name =
        hinge_ui_joint2_->occupant.ui_module->module_name;
    if (mutable_tip_mod_name != hinge2_mod_name) {
        // JUtil.warn(("Need fix!" + mutable_tip_mod_name +
        //             " vs " + hinge2_mod_name + "\n").c_str());
    }

    HingeTeam::evavluate();
}

void DoubleHingeTeam::virtual_implement_recipe(
    tests::Recipe const& recipe,
    FirstLastNodeKeyCallback const& _postprocessor,
    Transform const& shift_tx)
{
    // Partial reset.
    hinge_ui_joint2_ = nullptr;

    FirstLastNodeKeyCallback const& postprocessor =
    [&](NodeKey const first_node, NodeKey const last_node) {
        pimpl_->find_hinge2();

        if (_postprocessor) {
            _postprocessor(first_node, last_node);
        }
    };

    HingeTeam::virtual_implement_recipe(recipe, postprocessor, shift_tx);
}

/* public */
/* ctors */
DoubleHingeTeam::DoubleHingeTeam(WorkArea const* wa) :
    HingeTeam(wa),
    pimpl_(make_pimpl())
{
    DEBUG_NOMSG(wa->ptterm_profile.empty());
    pimpl_->find_hinge2();
}

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

/* modifiers */
DoubleHingeTeam& DoubleHingeTeam::operator=(DoubleHingeTeam const& other) {
    HingeTeam::operator=(other);
    hinge_ui_joint2_ = other.hinge_ui_joint2_;
    return *this;
}

DoubleHingeTeam& DoubleHingeTeam::operator=(DoubleHingeTeam && other) {
    HingeTeam::operator=(std::move(other));
    std::swap(hinge_ui_joint2_, other.hinge_ui_joint2_);
    return *this;
}

}  /* elfin */