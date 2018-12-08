#include "link.h"

namespace elfin {

bool Link::operator==(const Link & other) const {
    return src_chain == other.src_chain and
           dst_chain == other.dst_chain;
}

}  /* elfin */