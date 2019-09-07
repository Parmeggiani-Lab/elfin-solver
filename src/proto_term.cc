#include "proto_term.h"

#include <algorithm>
#include <deque>
#include <tuple>

#include "proto_module.h"
#include "debug_utils.h"
#include "exceptions.h"
#include "proto_module.h"

namespace elfin {

/* public */
/* accessors */
ProtoLink const& ProtoTerm::pick_random_link(
    TermType const term,
    uint32_t& seed) const
{
    if (term == TermType::N) {
        return *n_roulette_.draw(seed);
    }
    else if (term == TermType::C) {
        return *c_roulette_.draw(seed);
    }
    else {
        throw BadTerminus(TermTypeToCStr(term));
    }
}

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
PtLinkKey ProtoTerm::find_link_to(PtModKey const dst_module,
                                  size_t const dst_chain_id,
                                  TermType const term) const {
    ProtoLink const key_link(Transform(), dst_module, dst_chain_id, term);

    auto link_itr = link_set_.find(&key_link);

    if (link_itr == end(link_set_)) {
        return nullptr;
    }

    return *link_itr;
}

PtLinkKey ProtoTerm::find_link_to(PtTermKey const ptterm) const {
    for (auto const& link : links_) {
        if (&link->get_term() == ptterm)
            return link.get();
    }
    return nullptr;
}

bool ProtoTerm::get_nearest_path_to(PtTermKeys const& acceptables, PtLinkKeys& result) const {
    // Returns a nearest path to specified acceptable ProtoTerms. The order is in reverse!
    PtTermKeySet acceptable_set(begin(acceptables), end(acceptables));
    PtTermKeySet visited_set;
    // Key: inward key, value: tuple of link and previous inward key.
    std::unordered_map<PtTermKey, std::tuple<PtLinkKey, PtTermKey>> waypoints;

    // Frontier consists of outward ProtoTerm and the inward ProtoTerm that led to it.
    std::deque<std::tuple<PtTermKey, PtTermKey>> frontier = { std::make_tuple(this, nullptr) };
    auto const add_to_frontier = [&](PtTermKey const out_key, PtTermKey const in_key) {
        if (out_key != in_key and visited_set.find(out_key) == end(visited_set)) {
            frontier.push_back(std::make_tuple(out_key, in_key));
            visited_set.insert(out_key);
        }
    };

    while (not frontier.empty()) {
        // Consume front of queue.
        auto const [curr_out_key, curr_in_key] = frontier.front();
        frontier.pop_front();

        // Put all unvisited outward ProtoTerm on the next wave into frontier.
        for (auto const& link : curr_out_key->links()) {
            auto const new_in_key = &link->get_term();
            if (waypoints.find(new_in_key) == end(waypoints))
                waypoints[new_in_key] = std::make_tuple(link.get(), curr_in_key);

            // Stop if we arrived at an acceptable ProtoTerm.
            if (acceptable_set.find(new_in_key) != end(acceptable_set)) {
                result.clear();

                auto curr_key = new_in_key;
                while (curr_key != nullptr) {
                    auto const waypoint = waypoints[curr_key];
                    result.push_back(std::get<0>(waypoint));
                    curr_key = std::get<1>(waypoint);
                }
                return true;
            }

            for (auto const& chain : link->module->chains()) {
                add_to_frontier(&chain.n_term(), new_in_key);
                add_to_frontier(&chain.c_term(), new_in_key);
            }
        }
    }

    return false;
}

/* modifiers */
void ProtoTerm::configure(
    std::string const& mod_name,
    std::string const& chain_name,
    TermType const term) {
    n_roulette_.clear();
    c_roulette_.clear();
    link_set_.clear();

    if (not active_) return;

    //
    // Sort links by interface count in ascending order to facilitate fast
    // pick_random() that support partitioning by interface count.
    //
    std::sort(begin(links_),
              end(links_),
    [](auto const & lhs, auto const & rhs) {
        return lhs->module->counts().all_interfaces() <
               rhs->module->counts().all_interfaces();
    });

    for (auto& link : links_) {
        DEBUG_NOMSG(nullptr == link->module);

        auto const link_ptr = link.get();
        link_set_.insert(link_ptr);

        size_t n_cpd = 0, c_cpd = 0;

        // Let inactive termini have 0 probability of getting picked.
        if (link->get_term().is_active()) {
            auto const target_prot = link->module;
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

        n_roulette_.push_back(n_cpd, link_ptr);
        c_roulette_.push_back(c_cpd, link_ptr);
    }

    //
    // Compute checksum for terminal, which will be used in computing
    // candidate checksum
    //
    checksum_ = checksum_new((void*) mod_name.data(), mod_name.length());
    checksum_cascade(&checksum_, (void*) chain_name.data(), chain_name.length());
    checksum_cascade(&checksum_, &term, sizeof(term));
}

}  /* elfin */