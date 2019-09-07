#include "hinge_team.h"

#include "input_manager.h"
#include "path_generator.h"
#include "ui_joint.h"
#include "priv_impl.h"

namespace elfin {

/* private */
struct HingeTeam::PImpl : public PImplBase<HingeTeam> {
    using PImplBase::PImplBase;

    void place_hinge() {
        _.hinge_ui_joint_ = nullptr;
        _.hinge_ = nullptr;

        auto const& omap = _.work_area_->occupied_joints;

        // There can be 1 or 2 occupied joints. Allow size of 2 for
        // DoubleHingeTeam.
        auto const& [oname, ojoint] = random::pick(omap, _.seed_);
        _.hinge_ui_joint_ = ojoint;

        // Place hinge node.
        auto const& ui_mod = _.hinge_ui_joint_->occupant.ui_module;
        auto mod_key = XDB.get_mod(ui_mod->module_name);

        FreeTerm const* exclude_ft_ptr = nullptr;
        // A hinge may not necessarily have other connected modules.
        if (not ui_mod->linkage.empty()) {
            // TODO: Make add_node() take a list instead of just one FreeTerm exclusion pointer
            auto const occupying_link = *begin(ui_mod->linkage);
            auto const occuping_chain_id = mod_key->get_chain_id(occupying_link.src_chain_name);
            exclude_ft_ptr = new FreeTerm(nullptr, occuping_chain_id, occupying_link.term);
        }
        _.hinge_ = _.add_node(mod_key, ui_mod->tx, /*innert=*/false, 1, exclude_ft_ptr);
        delete exclude_ft_ptr;
    }

    UIJointKey find_ui_joint(tests::RecipeStep const& first_step) {
        auto const& omap = _.work_area_->occupied_joints;

        auto const itr = omap.find(first_step.ui_name);
        DEBUG(itr == end(omap),
              "Could not find UIJoint with name: %s\n",
              first_step.ui_name.c_str());

        return itr->second;
    }
};

/* protected */
/* accessors */
HingeTeam* HingeTeam::virtual_clone() const {
    return new HingeTeam(*this);
}

NodeKey HingeTeam::get_tip(bool const mutable_hint) const
{
    if (mutable_hint) {
        return begin(free_terms_)->node;
    }
    else {
        return hinge_;
    }
}

void HingeTeam::mutation_invariance_check() const {
    DEBUG_NOMSG(not hinge_);
    DEBUG_NOMSG(size() == 0);

    size_t const n_free_terms = free_terms_.size();
    if (n_free_terms != 1) {
        JUtil.error("Should only exactly one FreeTerm for HingeTeam.\n");
        for (auto& ft : free_terms_) {
            JUtil.error("%s\n", ft.to_string().c_str());
        }
        DEBUG(n_free_terms != 1, "n_free_terms=%zu", n_free_terms);
    }
}

bool HingeTeam::is_mutable(NodeKey const tip) const {
    return tip != hinge_;
}

void HingeTeam::postprocess_json(JSON& output) const {
    if (output.size() > 0) {
        // Skip first hinge.
        output.erase(0);
    }
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
    checksum_ = PathGenerator(hinge_).checksum();
}

void HingeTeam::calc_score() {
    score_ = INFINITY;

    // Collect points without hinge itself.
    auto const& my_points = PathGenerator(hinge_).collect_points();
    auto const& ref_path = work_area_->path_map.at(hinge_ui_joint_);

    score_ = score_func_(my_points, ref_path);

    scored_path_ = &ref_path;
}

void HingeTeam::virtual_implement_recipe(
    tests::Recipe const& recipe,
    FirstLastNodeKeyCallback const& _postprocessor,
    Transform const& shift_tx)
{
    // Partial reset: do not place hinge.
    PathTeam::reset();
    hinge_ui_joint_ = pimpl_->find_ui_joint(recipe.at(0));
    hinge_ = nullptr;

    FirstLastNodeKeyCallback const& postprocessor =
    [&](NodeKey const first_node, NodeKey const last_node) {
        hinge_ = first_node;

        // PathTeam::virtual_implement_recipe() adds free terms from both
        // tips, so the *one free term* belonging to hinge_ needs to be
        // removed.
        remove_free_terms(hinge_);

        if (_postprocessor) {
            _postprocessor(first_node, last_node);
        }
    };

    PathTeam::virtual_implement_recipe(recipe, postprocessor, shift_tx);
}

/* public */
/* ctors */
HingeTeam::HingeTeam(WorkArea const* const wa,
                     uint32_t const seed,
                     bool const loose) :
    PathTeam(wa, seed),
    pimpl_(new_pimpl<PImpl>(*this)),
    score_func_(loose ?
                scoring::score_aligned :
                scoring::score_unaligned)
{
    // Call place_hinge() after initializer list because hinge_ needs to be
    // initialiezd as nullptr.
    pimpl_->place_hinge();
    mutation_invariance_check();
}

HingeTeam::HingeTeam(HingeTeam const& other) :
    HingeTeam(other.work_area_, other.seed_)
{ HingeTeam::operator=(other); }

HingeTeam::HingeTeam(HingeTeam&& other)  :
    HingeTeam(other.work_area_, other.seed_)
{ HingeTeam::operator=(std::move(other)); }

/* dtors */
HingeTeam::~HingeTeam() {}

/* modifiers */
HingeTeam& HingeTeam::operator=(HingeTeam const& other) {
    PathTeam::operator=(other);
    hinge_ui_joint_ = other.hinge_ui_joint_;
    hinge_ = other.hinge_ ? nk_map_.at(other.hinge_) : nullptr;
    score_func_ = other.score_func_;
    return *this;
}

HingeTeam& HingeTeam::operator=(HingeTeam && other) {
    PathTeam::operator=(std::move(other));
    std::swap(hinge_ui_joint_, other.hinge_ui_joint_);
    std::swap(hinge_, other.hinge_);
    std::swap(score_func_, other.score_func_);
    return *this;
}

}  /* elfin */