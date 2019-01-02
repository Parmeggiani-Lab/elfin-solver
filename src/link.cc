#include "link.h"

#include "node.h"
#include "path_generator.h"

namespace elfin {

/* public */
/* ctors */
Link::Link(FreeTerm const& src,
           ProtoLink const* prot,
           FreeTerm const& dst) :
  src_(src),
  prototype_(prot),
  dst_(dst)
{
  // Check that prototype_ (ProtoLink) destination ProtoModule is the same
  // as dst ProtoModule.
  DEBUG(dst_.node->prototype_ != prototype_->module_,
        "\ndst_.node->prototype_=%s\nprototype_->module_=%s\n",
        dst_.node->prototype_->to_string().c_str(),
        prototype_->module_->to_string().c_str());
}

/* accessors */
// bool Link::operator==(Link const& other) const {
//     return src == other.src and
//            dst == other.dst();
// }

PathGenerator Link::gen_path() const {
  return PathGenerator(this);
}

/* modifiers */
void Link::update_node_ptrs(NodeKeyMap const& nam) {
  src_.node = nam.at(src_.node);
  dst_.node = nam.at(dst_.node);
}

/* printers */
void Link::print_to(std::ostream& os) const {
  os << "Link[\n  " << src_;
  os << "\n  " << dst_ << "\n]";
}

}  /* elfin */