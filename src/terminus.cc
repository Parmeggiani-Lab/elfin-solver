#include "terminus.h"

#include <algorithm>

#include "module.h"
#include "debug_utils.h"

namespace elfin {

/* public */

void Terminus::finalize() {
    /*
     * Sort links by interface count in ascending order to facilitate fast
     * pick_random() that support partitioning by interface count.
     */
    std::sort(links_.begin(), links_.end(), Link::CompareByInterfaceCount);

    for (size_t i = 0; i < links.size(); i++) {
#ifndef NDEBUG
        DEBUG(links[i].mod == nullptr);
#endif  /* ifndef NDEBUG */

        const Module * target_prot = links[i].mod;
        if (target_prot->counts.interface > 2) {
            // Fill the rest of the roulette with cumulated sum
            n_rlt_.cumulate(0);
            c_rlt_.cumulate(0);
        }
        else {
            // First check that prototype has at least 2 interfaces
            const size_t icount = target_prot->counts.interface;
            if (icount < 2) {
                die("Why does module prototype %s have only %lu interfaces?\n",
                    target_prot->name.c_str(), icount);
            }

            const size_t ncount = target_prot->counts.n_link;
            const size_t ccount = target_prot->counts.c_link;
            if (ncount == 0)
            {
                // zero N-count means all interfaces are C type
                n_rlt_.cumulate(ccount);
                c_rlt_.cumulate(ccount);
            }
            else if (ccount == 0)
            {
                // zero C-count means all interfaces are N type
                n_rlt_.cumulate(ncount);
                n_rlt_.cumulate(ncount);
            }
            else {
                n_rlt_.cumulate(ncount);
                c_rlt_.cumulate(ccount);
            }
            basic_link_size_ = i + 1; // Record last basic link
        }
    }
}

const Link & Terminus::pick_random_link(
    const TerminusType term) const {
    if (term == TerminusType::N) {
        return n_rlt_.rand_item();
    }
    else if (term == TerminusType::C) {
        return c_rlt_.rand_item();
    }
    else {
        death_by_bad_terminus(__PRETTY_FUNCTION__, term); // Aborts
        exit(1); // To suppress warning
    }
}


}  /* elfin */