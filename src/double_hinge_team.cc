#include "double_hinge_team.h"

#include <unordered_map>
#include <unordered_set>

#include "input_manager.h"
#include "path_generator.h"
#include "priv_impl.h"
#include "proto_link.h"

namespace elfin {

/* private */
struct DoubleHingeTeam::PImpl : public PImplBase<DoubleHingeTeam> {
    using PImplBase::PImplBase;

    void find_hinge2() {
        DEBUG_NOMSG(not _.hinge_ui_joint_);
        DEBUG_NOMSG(not _.hinge_);

        auto const& omap = _.work_area_->occupied_joints;

        auto const itr = find_if(begin(omap), end(omap),
        [&](auto const & omap_pair) {
            return omap_pair.second != _.hinge_ui_joint_;
        });
        DEBUG_NOMSG(itr == end(omap));

        _.hinge_ui_joint2_ = itr->second;
    }

    bool complete_path() {
        DEBUG_NOMSG(not _.hinge_ui_joint2_);

        // Gather src info
        auto const src_fterm = _.get_mutable_term();
        auto const src_mod = src_fterm.node->prototype_;
        PtTermKey const src_ptterm = &src_mod->get_term(src_fterm);

        // Gather dst info
        auto const dst_uimod = _.hinge_ui_joint2_->occupant.ui_module;
        auto const dst_mod = XDB.get_mod(dst_uimod->module_name);
        auto const dst_ptterms = dst_uimod->free_ptterms;

        // Termination condition is when the frontier contains at least one
        // ProtoTerm that is accepted by the dst.
        for (auto const& acceptable : dst_ptterms)
            if (src_ptterm->find_link_to(acceptable))
                return true;

        // Invoke dijkstra
        PtLinkKeys nearest_path;
        if (src_ptterm->get_nearest_path_to(dst_ptterms, nearest_path)) {
            // Check path sanity. Note the path is reversed.
            DEBUG_NOMSG(nearest_path.back()->reverse->module != src_mod);
            DEBUG_NOMSG(nearest_path.front()->module != dst_mod);

            // Grow the links.
            for (auto itr = nearest_path.rbegin();
                    itr != nearest_path.rend(); ++itr) {
                _.grow_tip(_.get_mutable_term(), *itr);
                DEBUG_NOMSG(_.free_terms_.size() != 1);
            }

            return true;
        }

        // Coud not reach dest hinge. It happens.
        return false;
    }
};

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
void DoubleHingeTeam::evaluate() {
    // Run djistrak to complete the mutable end if team does not end in second
    // hinge.
    bool ok = pimpl_->complete_path();
    HingeTeam::evaluate();
    if (!ok)
        score_ = INFINITY;
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
DoubleHingeTeam::DoubleHingeTeam(WorkArea const* const wa,
                                 uint32_t const seed) :
    HingeTeam(wa, seed),
    pimpl_(new_pimpl<PImpl>(*this))
{
    DEBUG_NOMSG(wa->ptterm_profile.empty());
    pimpl_->find_hinge2();
}

DoubleHingeTeam::DoubleHingeTeam(DoubleHingeTeam const& other) :
    DoubleHingeTeam(other.work_area_, other.seed_) {
    HingeTeam::operator=(other);

}
DoubleHingeTeam::DoubleHingeTeam(DoubleHingeTeam&& other) :
    DoubleHingeTeam(other.work_area_, other.seed_) {
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