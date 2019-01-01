#include "double_hinge_team.h"

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
// DoubleHingeTeam* DoubleHingeTeam::virtual_clone() const {
// }

/* modifiers */
// void DoubleHingeTeam::virtual_copy(NodeTeam const& other) {
// }

/*public*/
/* ctors */
DoubleHingeTeam::DoubleHingeTeam(WorkArea const* wa) :
    NodeTeam(wa),
    pimpl_(make_pimpl()) {}

// DoubleHingeTeam::DoubleHingeTeam(DoubleHingeTeam const& other){}
// DoubleHingeTeam::DoubleHingeTeam(DoubleHingeTeam&& other){}

/* dtors */
DoubleHingeTeam::~DoubleHingeTeam() {}

/* accessors */
// size_t DoubleHingeTeam::size() const {
// }

// PathGenerator DoubleHingeTeam::gen_path() const {
// }

/* modifiers */
// DoubleHingeTeam& DoubleHingeTeam::operator=(DoubleHingeTeam const& other){}
// DoubleHingeTeam& DoubleHingeTeam::operator=(DoubleHingeTeam && other){}
// void DoubleHingeTeam::randomize() {
// }

// mutation::Mode DoubleHingeTeam::evolve(NodeTeam const& mother,
//                                        NodeTeam const& father)
// {
// }

// void DoubleHingeTeam::implement_recipe(
//     tests::Recipe const& recipe,
//     Transform const& shift_tx))
// {
// }

/* printers */
// void DoubleHingeTeam::print_to(std::ostream& os) const {
// }

// JSON DoubleHingeTeam::to_json() const {
// }

}  /* elfin */