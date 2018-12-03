#include "chain.h"

#include <algorithm>

namespace elfin {

/* public */
const LinkList & Chain::get_links(const TerminusType term) const {
    if (term == TerminusType::N) {
        return n_links;
    }
    else if (term == TerminusType::C) {
        return c_links;
    }
    else {
        death_by_bad_terminus(__PRETTY_FUNCTION__, term);
    }
}

void Chain::finalize() {
    /*
     * Sort links by interface count in ascending order to facilitate fast
     * pick_random()
     */
    std::sort(n_links_.begin(), n_links_.end(), Link::InterfaceComparator);
    std::sort(c_links_.begin(), c_links_.end(), Link::InterfaceComparator);
}

const Link & Chain::pick_random_link() const {
    die("not impl\n");
}

}  /* elfin */