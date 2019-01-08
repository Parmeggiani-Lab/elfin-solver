#include "proto_link.h"

#include "proto_module.h"
#include "proto_term.h"

namespace elfin {

/* accessors */
ProtoTerm const& ProtoLink::get_term() const {
  return module->chains().at(chain_id).get_term(term);
};

/* printers */
void ProtoLink::print_to(std::ostream& os) const {
  os << "ProtoLink (" << module->name << ") [\n";
  os << "  Chain ID: " << chain_id << "\n";
  os << "  Term: " << TermTypeToCStr(term) << "\n";
  os << tx << "\n";
  os << "]";
}

size_t HashPtLinkSimple::operator()(PtLinkKey const& link) const {
  return std::hash<void *>()((void *) link->module) ^
         std::hash<size_t>()(link->chain_id) ^
         std::hash<TermType>()(link->term);
}

bool EqualPtLinkSimple::operator()(
  PtLinkKey const& lh_link,
  PtLinkKey const& rh_link) const
{
  return lh_link->module == rh_link->module and
         lh_link->chain_id == rh_link->chain_id and
         lh_link->term == rh_link->term;
}


}  /* elfin */