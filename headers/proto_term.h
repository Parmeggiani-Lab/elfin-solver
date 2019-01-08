#ifndef PROTO_TERM_H_
#define PROTO_TERM_H_

#include <vector>
#include <unordered_set>

#include "proto_link.h"
#include "term_type.h"
#include "roulette.h"

namespace elfin {

/* Fwd Decl */
class ProtoModule;
class ProtoTerm;
typedef ProtoTerm const* PtTermKey;
typedef std::vector<PtTermKey> PtTermKeys;
typedef std::unordered_set<PtTermKey> PtTermKeySet;

/* types */

typedef ProtoModule const* PtModKey;

class ProtoTerm {
    friend ProtoModule;
private:
    /* types */
    typedef Roulette<PtLinkKey> PtLinkRoulette;

    // Note: with c++20, std::unordered_set::find(K& key) becomes a template
    // method. We'll be able to search the pointer set without creating a new
    // ProtoLink instance (by comparing ProtoLink* with FreeTerm *).
    typedef std::unordered_set <PtLinkKey,
            HashPtLinkSimple,
            EqualPtLinkSimple > PtLinkKeySet;

    /* data */
    bool already_finalized_ = false;
    PtLinks links_;
    PtLinkRoulette n_roulette_, c_roulette_;
    PtLinkKeySet link_set_;

public:
    /* ctors */
    ProtoTerm() {}
    ProtoTerm(ProtoTerm const& other) = delete;
    ProtoTerm(ProtoTerm&& other) = delete;
    ProtoTerm& operator=(ProtoTerm const& other) = delete;
    ProtoTerm& operator=(ProtoTerm&& other) = delete;

    /* accessors */
    PtLinks const& links() const { return links_; }
    ProtoLink const& pick_random_link(TermType const term) const;
    PtLinkKeySet const& link_set() const { return link_set_; }
    PtLinkKey find_link_to(PtModKey const dst_module,
                           size_t const dst_chain_id,
                           TermType const term) const;

    /* modifiers */
    void finalize();
};

}  /* elfin */

#endif  /* end of include guard: PROTO_TERM_H_ */