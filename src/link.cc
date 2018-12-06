#include "link.h"

#include "module.h"

namespace elfin {

bool Link::operator<(const Link & rhs) const {
    return mod->counts().interface < rhs.mod->counts().interface;
}

}  /* elfin */