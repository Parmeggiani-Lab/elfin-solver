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

    void place_hinge() {
        _.hinge_ui_joint_ = nullptr;
        _.hinge_ = nullptr;

        auto const& omap = _.work_area_->occupant_map;

        // There can be 1 or 2 occupied joints. Allow size of 2 for
        // DoubleHingeTeam.
        auto const& [oname, ojoint] = random::pick(omap);
        _.hinge_ui_joint_ = ojoint;

        // Place hinge node.
        auto const& ui_mod = _.hinge_ui_joint_->occupant.ui_module;
        auto proto_mod = XDB.get_module(ui_mod->module_name);

        // Here, if elfin-ui provides info about which specific chain of
        // the hinge module interfaces with the next unknown module
        // (joint), then we could remove free chains that are outside of
        // this work area. However right now that functionality is not
        // implemented by elfin-ui so we'll leave the selection to GA.

        // Do not add any free chain. Let get_mutable_chain() handle the case
        // where the team consists of only hinge_.
        _.hinge_ = _.add_node(proto_mod, ui_mod->tx);
    }

    UIJointKey find_ui_joint(tests::RecipeStep const& first_step) {
        auto const& omap = _.work_area_->occupant_map;

        auto itr = omap.find(first_step.ui_name);
        TRACE_NOMSG(itr == end(omap));

        return itr->second;
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

FreeChain HingeTeam::get_mutable_chain() const
{
    // If the only node is the hinge, then return random free chain from hinge.
    if (size() == 1) {
        auto const& rand_hinge_fc = random::pick(hinge_->prototype_->free_chains());
        return FreeChain(hinge_, rand_hinge_fc.term, rand_hinge_fc.chain_id);
    }
    else {
        // Return a random free chain from mutable tip.
        return PathTeam::get_mutable_chain();
    }
}

NodeKey HingeTeam::get_tip(bool const mutable_hint) const {
    if (mutable_hint) {
        DEBUG(size() == 1, "Cannot request mutable tip when team has only hinge.\n");

        DEBUG_NOMSG(free_chains_.size() != 1);

        // Return the only other node, which all (only one) free chains
        // currently belong to.
        return begin(free_chains_)->node;
    }
    else {
        return hinge_;
    }
}

void HingeTeam::mutation_invariance_check() const {
    DEBUG_NOMSG(not hinge_);
    DEBUG_NOMSG(size() == 0);

    size_t const n_free_chains = free_chains_.size();
    if (size() == 1) {
        // There are no "free chains" when only hinge exists.
        DEBUG_NOMSG(n_free_chains != 0);
    }
    else {
        // There are always exactly one free chain for size() > 1.
        DEBUG_NOMSG(n_free_chains != 1);
    }
}

bool HingeTeam::is_mutable(NodeKey const tip) const {
    return tip != hinge_;
}

/* modifiers */
void HingeTeam::reset() {
    PathTeam::reset();
    hinge_ui_joint_ = nullptr;
    hinge_ = nullptr;

    pimpl_->place_hinge();
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
    checksum_ = hinge_->gen_path().path_checksum();
}

void HingeTeam::calc_score() {
    score_ = INFINITY;

    // Collect points without hinge itself.
    auto const& my_points = hinge_->gen_path().collect_points();

    // Should use simple_rms(), but there's some floating point rounding error
    // that's causing unit tests to fail. If we used inner_product() in
    // simple_rms(), that'd yield the satisfactory result but then an extra vector is
    // required. Until transform_reduce() is supported, Kabsch RMS should be
    // fine performance-wise?
    auto const& ref_path = work_area_->path_map.at(hinge_ui_joint_);
    score_ = scoring::score(my_points, ref_path);

    scored_path_ = &ref_path;
}

void HingeTeam::virtual_implement_recipe(
    tests::Recipe const& recipe,
    NodeKeyCallback const& cb_on_first_node,
    Transform const& shift_tx)
{
    // Partial reset: do not place hinge.
    PathTeam::reset();
    hinge_ui_joint_ = pimpl_->find_ui_joint(recipe.at(0));
    hinge_ = nullptr;

    auto const& cb =
    NodeKeyCallback([&](NodeKey nk) {
        if (cb_on_first_node) {
            cb_on_first_node(nk);
        }

        hinge_ = nk;

        // PathTeam::implement_recipe() uses add_free_node(), so free chains
        // belonging to hinge_ need to be cleared.
        free_chains_.clear();
    });

    PathTeam::virtual_implement_recipe(recipe, cb, shift_tx);
}

/* public */
/* ctors */
HingeTeam::HingeTeam(WorkArea const* wa) :
    PathTeam(wa),
    pimpl_(make_pimpl())
{
    // Call place_hinge() after initializer list because hinge_ needs to be
    // initialiezd as nullptr.
    pimpl_->place_hinge();
    mutation_invariance_check();
}

HingeTeam::HingeTeam(HingeTeam const& other) :
    HingeTeam(other.work_area_)
{ HingeTeam::operator=(other); }

HingeTeam::HingeTeam(HingeTeam&& other)  :
    HingeTeam(other.work_area_)
{ HingeTeam::operator=(std::move(other)); }

/* dtors */
HingeTeam::~HingeTeam() {}

/* modifiers */
HingeTeam& HingeTeam::operator=(HingeTeam const& other) {
    PathTeam::operator=(other);
    hinge_ui_joint_ = other.hinge_ui_joint_;
    hinge_ = other.hinge_ ? nk_map_.at(other.hinge_) : nullptr;
    return *this;
}

HingeTeam& HingeTeam::operator=(HingeTeam && other) {
    PathTeam::operator=(std::move(other));
    std::swap(hinge_ui_joint_, other.hinge_ui_joint_);
    std::swap(hinge_, other.hinge_);
    return *this;
}

}  /* elfin */