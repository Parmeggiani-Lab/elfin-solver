#include "terminus.h"

#include <algorithm>

#include "module.h"
#include "debug_utils.h"

// #define PRINT_FINALIZE

namespace elfin {

/* public */

void Terminus::finalize() {
    NICE_PANIC(finalized_,
               string_format("%s called more than once!", __PRETTY_FUNCTION__).c_str());
    finalized_ = true;

#ifdef PRINT_FINALIZE
    wrn("Finalizing term with %lu links\n", links_.size());
#endif  /* ifdef PRINT_FINALIZE */
    /*
     * Sort links by interface count in ascending order to facilitate fast
     * pick_random() that support partitioning by interface count.
     */
    std::sort(links_.begin(), links_.end());

    std::vector<float> n_cpd, c_cpd;
    std::vector<Link *> ptrs;
    for (auto itr = links_.begin();
            itr != links_.end();
            itr++) {
        Link & link = *itr;
        DEBUG(nullptr == link.mod);

        const Module * target_prot = link.mod;
        if (target_prot->counts().interface > 2) {
            // Fill the rest of the roulette with total probability (can't be
            // picked by rand_item())
            n_cpd.push_back(0);
            c_cpd.push_back(0);
        }
        else {
            // First check that prototype has at least 2 interfaces
            const size_t ncount = target_prot->counts().n_link;
            const size_t ccount = target_prot->counts().c_link;
            if (ncount == 0)
            {
                // zero N-count means all interfaces are C type
                n_cpd.push_back(ccount);
                c_cpd.push_back(ccount);
            }
            else if (ccount == 0)
            {
                // zero C-count means all interfaces are N type
                n_cpd.push_back(ncount);
                c_cpd.push_back(ncount);
            }
            else {
                n_cpd.push_back(ncount);
                c_cpd.push_back(ccount);
            }
            basic_link_size_ = itr - links_.begin(); // Record last basic link
        }

        ptrs.push_back(&link);
#ifdef PRINT_FINALIZE
        wrn("Link to %s with %lu interfaces\n",
            link.mod->name.c_str(),
            link.mod->counts().interface);
#endif  /* ifdef PRINT_FINALIZE */
    }

    n_rlt_ = Roulette<Link *>(ptrs, n_cpd);
    c_rlt_ = Roulette<Link *>(ptrs, c_cpd);
}

const Link & Terminus::pick_random_link(
    const TerminusType term) const {
    if (term == TerminusType::N) {
        return *n_rlt_.draw();
    }
    else if (term == TerminusType::C) {
        return *c_rlt_.draw();
    }
    else {
        death_by_bad_terminus(__PRETTY_FUNCTION__, term); // Aborts
        exit(1); // To suppress warning
    }
}


}  /* elfin */