#include "proto_terminus.h"

#include <algorithm>

#include "proto_module.h"
#include "debug_utils.h"

// #define PRINT_FINALIZE

namespace elfin {

/* public */
/* accessors */
ProtoLink const& ProtoTerminus::pick_random_link(
    TerminusType const term) const {
    if (term == TerminusType::N) {
        return *n_roulette_.draw();
    }
    else if (term == TerminusType::C) {
        return *c_roulette_.draw();
    }
    else {
        bad_terminus(term);
        exit(1); // Suppress no return warning.
    }
}

//
// find_link_to()
//  - This assumes that links are identical as long as their module and
//    chain_id are identical. The transformation matrix does not need to
//    be compared.
//  - There should be either one or no ProtoLink that meets the search
//    criteria. A ProtoLink connects exactly one N terminus and one C
//    terminus between the src and dst ProtoModules. On any given chain,
//    there is exactly one N and one C.
//
// In c++20 we could search without creating a new instance, by
// implementing specialized comparators with custom key type.
//
ProtoLinkPtrSetCItr ProtoTerminus::find_link_to(
    ConstProtoModulePtr dst_module,
    size_t const dst_chain_id) const {
    ProtoLink const key_link(Transform(), dst_module, dst_chain_id);
    return link_set_.find(&key_link);
}


/* modifiers */
void ProtoTerminus::finalize() {
    NICE_PANIC(finalized_,
               string_format("%s called more than once!", __PRETTY_FUNCTION__).c_str());
    finalized_ = true;

#ifdef PRINT_FINALIZE
    JUtil.warn("Finalizing proto terminus with %zu links\n", links_.size());
#endif  /* ifdef PRINT_FINALIZE */

    //
    // Sort links by interface count in ascending order to facilitate fast
    // pick_random() that support partitioning by interface count.
    //
    std::sort(
        begin(links_),
        end(links_),
    [](ProtoLinkSP const & lhs, ProtoLinkSP const & rhs) {
        return lhs->module_->counts().all_interfaces() <
               rhs->module_->counts().all_interfaces();
    });

    for (auto& link : links_) {
        DEBUG(nullptr == link->module_);

        ProtoLink const* row_link_ptr = link.get();
        link_set_.insert(row_link_ptr);


        //
        // Note: assigning 0 probability for ProtoLinks that have more than 2
        // interfaces.
        //
        // The reason for doing this is that in the current paradigm we only
        // work with simple path candidates. In order to select a valid basic
        // proto module in O(1), we ignore anything that has more than 2
        // interfaces (which are all hubs). Some hubs have only 2 interfaces
        // and can be used to reverse terminus polarity while maintaining a
        // simple path shape.
        //
        // If progress down the road comes to dealing with generalized shape
        // candidates, it might be advisable to remove this restriction so
        // hubs > 2 interfaces can also be drawn from a ProtoModule's
        // ProtoLinks.
        //
        size_t n_cpd = 0, c_cpd = 0;

        ProtoModule const* target_prot = link->module_;
        if (target_prot->counts().all_interfaces() <= 2) {
            size_t const ncount = target_prot->counts().n_links;
            size_t const ccount = target_prot->counts().c_links;

            if (ncount == 0)
            {
                // zero N-count means all interfaces are C type
                n_cpd = ccount;
                c_cpd = ccount;
            }
            else if (ccount == 0)
            {
                // zero C-count means all interfaces are N type
                n_cpd = ncount;
                c_cpd = ncount;
            }
            else {
                n_cpd = ncount;
                c_cpd = ccount;
            }
        }

        n_roulette_.push_back(n_cpd, row_link_ptr);
        c_roulette_.push_back(c_cpd, row_link_ptr);

#ifdef PRINT_FINALIZE
        JUtil.warn("ProtoLink to %s into chain %zu with %zu interfaces\n",
            link_ptr->module_->name.c_str(),
            link_ptr->chain_id_,
            link_ptr->module_->counts().all_interfaces());
#endif  /* ifdef PRINT_FINALIZE */
    }
}
}  /* elfin */