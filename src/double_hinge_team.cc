#include "double_hinge_team.h"

#include "input_manager.h"

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

void DoubleHingeTeam::add_node_check(ProtoModule const* const prot) const {
    // Allow any module.
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

/*public*/
/* ctors */
DoubleHingeTeam::DoubleHingeTeam(WorkArea const* wa) :
    HingeTeam(wa),
    pimpl_(make_pimpl())
{
    mutation_invariance_check();
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