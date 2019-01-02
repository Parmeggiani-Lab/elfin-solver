#include "proto_term.h"

#include <algorithm>

#include "proto_module.h"
#include "debug_utils.h"
#include "exit_exception.h"

// #define PRINT_FINALIZE

namespace elfin {

/* public */
/* accessors */
ProtoLink const& ProtoTerm::pick_random_link(
    TermType const term) const {
    if (term == TermType::N) {
        return *n_roulette_.draw();
    }
    else if (term == TermType::C) {
        return *c_roulette_.draw();
    }
    else {
        bad_term(term);
        throw ExitException{1};  // Suppress no return warning.
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
ProtoLinkPtrSetCItr ProtoTerm::find_link_to(
    ConstProtoModulePtr dst_module,
    size_t const dst_chain_id) const {
    ProtoLink const key_link(Transform(), dst_module, dst_chain_id);
    return link_set_.find(&key_link);
}


/* modifiers */
void ProtoTerm::finalize() {
    TRACE_NOMSG(already_finalized_);
    already_finalized_ = true;

#ifdef PRINT_FINALIZE
    JUtil.warn("Finalizing ProtoTerm with %zu links\n", links_.size());
#endif  /* ifdef PRINT_FINALIZE */

    //
    // Sort links by interface count in ascending order to facilitate fast
    // pick_random() that support partitioning by interface count.
    //
    std::sort(
        begin(links_),
        end(links_),
    [](auto const & lhs, auto const & rhs) {
        return lhs->module_->counts().all_interfaces() <
               rhs->module_->counts().all_interfaces();
    });

    for (auto& link : links_) {
        DEBUG_NOMSG(nullptr == link->module_);

        ProtoLink const* row_link_ptr = link.get();
        link_set_.insert(row_link_ptr);

        size_t n_cpd = 0, c_cpd = 0;

        ProtoModule const* target_prot = link->module_;
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