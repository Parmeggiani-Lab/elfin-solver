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
void DoubleHingeTeam::evavluate() {
    // Run djistrak to complete the mutable end if team does not end in second
    // hinge.
    auto const mutable_tip = get_tip(/*mutable_hint=*/true);
    // if(mutable_tip != second_hinge_) {

    // }

    HingeTeam::evavluate();
}

/* public */
/* ctors */
DoubleHingeTeam::DoubleHingeTeam(WorkArea const* wa) :
    HingeTeam(wa),
    pimpl_(make_pimpl())
{
    DEBUG_NOMSG(wa->ptterm_profile.empty());
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