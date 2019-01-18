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
typedef ProtoModule const* PtModKey;

class ProtoTerm;
typedef ProtoTerm const* PtTermKey;
typedef std::vector<PtTermKey> PtTermKeys;
typedef std::unordered_set<PtTermKey> PtTermKeySet;

class ProtoTerm {
    friend ProtoModule;
private:
    /* types */
    typedef Roulette<PtLinkKey> PtLinkRoulette;

    // Note: with c++20, std::unordered_set::find(K& key) becomes a template
    // method. We'll be able to search the pointer set without creating a new
    // ProtoLink instance (by comparing ProtoLink* with FreeTerm *).
    typedef std::unordered_set <PtLinkKey,
            ProtoLink::PtLinkSimpleHasher,
            ProtoLink::PtLinkSimpleComparer > PtLinkKeySet;

    /* data */
    PtLinks links_;
    PtLinkRoulette n_roulette_, c_roulette_;
    PtLinkKeySet link_set_;
    bool active_ = true;

public:
    /* ctors */
    ProtoTerm() {}
    ProtoTerm(ProtoTerm const& other) = delete;
    ProtoTerm(ProtoTerm&& other) = delete;
    ProtoTerm& operator=(ProtoTerm const& other) = delete;
    ProtoTerm& operator=(ProtoTerm&& other) = delete;

    /* accessors */
    PtLinks const& links() const { return links_; }
    ProtoLink const& pick_random_link(TermType const term, uint32_t& seed) const;
    PtLinkKeySet const& link_set() const { return link_set_; }
    PtLinkKey find_link_to(PtModKey const dst_module,
                           size_t const dst_chain_id,
                           TermType const term) const;
    bool is_active() const { return active_; }

    /* modifiers */
    void configure();
    void activate() { active_ = true; }
    void deactivate() { active_ = false; }
};

}  /* elfin */


namespace std {
template <>
struct hash<elfin::ProtoTerm*>
{
    size_t operator()(elfin::ProtoTerm* const& key) const {
        return hash<void*>()((void*) key);
    }
};
}

namespace elfin {

/* more types */
struct PtTermFinder {
    /* types */
    struct hasher {
        size_t operator()(PtTermFinder const& f) const {
            return std::hash<ProtoTerm*>()(f.ptterm_ptr);
        }
    };
    struct comparer {
        size_t operator()(PtTermFinder const& lhs, PtTermFinder const& rhs) const {
            return lhs.ptterm_ptr == rhs.ptterm_ptr;
        }
    };

    /* data */
    PtModKey mod;
    size_t chain_id;
    TermType term;
    ProtoTerm* ptterm_ptr;

    PtTermFinder(PtModKey const _mod,
                 size_t const _chain_id,
                 TermType const _term,
                 ProtoTerm const* const _ptterm_ptr) :
        mod(_mod),
        chain_id(_chain_id),
        term(_term),
        ptterm_ptr(const_cast<ProtoTerm*>(_ptterm_ptr)) {}
};
typedef std::unordered_set <PtTermFinder, PtTermFinder::hasher, PtTermFinder::comparer> PtTermFinderSet;

}  /* elfin */

#endif  /* end of include guard: PROTO_TERM_H_ */