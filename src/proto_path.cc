#include "proto_path.h"

#include "proto_module.h"

namespace elfin {

/* printers */
void ProtoPath::print_to(std::ostream& os) const {
    os << "ProtoPath[";
    os << "  Start: " << start->name << "\n";
    for (auto const& ptlink : links) {
        os << "  PtLink: " << *ptlink << "\n";
    }
    os << "]";
}

}  /* elfin */