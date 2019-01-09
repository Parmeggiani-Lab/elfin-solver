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

/* public */
/* ctors */
DoubleHingeTeam::DoubleHingeTeam(WorkArea const* wa) :
    HingeTeam(wa),
    pimpl_(make_pimpl())
{
    DEBUG_NOMSG(wa->ptterm_profile.empty());
    // for (auto const& finder : XDB.ptterm_finders()) {
    //     if (not finder.ptterm_ptr->is_active()) {
    //         JUtil.error("Not active: %s.%s.%s\n",
    //                     finder.mod->name.c_str(),
    //                     finder.mod->get_chain(finder.chain_id).name.c_str(),
    //                     TermTypeToCStr(finder.term));
    //     }
    // }
    // JUtil.error("wa->ptterm_profile.size()=%zu, XDB.ptterm_finders().size()=%zu\n",
    //             wa->ptterm_profile.size(), XDB.ptterm_finders().size());
    // PANIC("Yo.");
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

}  /* elfin */