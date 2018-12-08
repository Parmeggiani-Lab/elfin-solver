#include "proto_terminus.h"

#include <algorithm>

#include "proto_module.h"
#include "debug_utils.h"

// #define PRINT_FINALIZE

namespace elfin {

/* public */

void ProtoTerminus::finalize() {
    NICE_PANIC(finalized_,
               string_format("%s called more than once!", __PRETTY_FUNCTION__).c_str());
    finalized_ = true;

#ifdef PRINT_FINALIZE
    wrn("Finalizing term with %lu links\n", proto_links_.size());
#endif  /* ifdef PRINT_FINALIZE */
    /*
     * Sort links by interface count in ascending order to facilitate fast
     * pick_random() that support partitioning by interface count.
     */
    std::sort(proto_links_.begin(), proto_links_.end());

    std::vector<float> n_cpd, c_cpd;
    std::vector<ProtoLink *> ptrs;

    for (auto itr = proto_links_.begin();
            itr != proto_links_.end();
            itr++) {
        
        ProtoLink & proto_link = *itr;
        DEBUG(nullptr == proto_link.target_mod);

        const ProtoModule * target_prot = proto_link.target_mod;

        /*
         * Note: assigning 0 probability for ProtoLinks that have more than 2
         * interfaces.
         *
         * The reason for doing this is that in the current paradigm we only
         * work with simple path candidates. In order to select a valid basic
         * proto module in O(1), we ignore anything that has more than 2
         * interfaces (which are all hubs). Some hubs have only 2 interfaces
         * and can be used to reverse terminus polarity while maintaining a
         * simple path shape.
         *
         * If progress down the road comes to dealing with generalized shape
         * candidates, it might be advisable to remove this restriction so
         * hubs > 2 interfaces can also be drawn from a ProtoModule's
         * ProtoLinks.
         */
        if (target_prot->counts().all_interfaces() > 2) {
            // Fill the rest of the roulette with total probability (can't be
            // picked by rand_item())
            n_cpd.push_back(0);
            c_cpd.push_back(0);
        }
        else {
            const size_t ncount = target_prot->counts().n_links;
            const size_t ccount = target_prot->counts().c_links;
            
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
        }

        ptrs.push_back(&proto_link);
#ifdef PRINT_FINALIZE
        wrn("ProtoLink to %s with %lu interfaces\n",
            proto_link.target_mod->name.c_str(),
            proto_link.target_mod->counts().all_interfaces());
#endif  /* ifdef PRINT_FINALIZE */
    }

    n_rlt_ = Roulette<ProtoLink *>(ptrs, n_cpd);
    c_rlt_ = Roulette<ProtoLink *>(ptrs, c_cpd);
}

const ProtoLink & ProtoTerminus::pick_random_proto_link(
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