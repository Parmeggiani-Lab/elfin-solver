#include "proto_module.h"

#include <sstream>
#include <memory>
#include <deque>
#include <functional>

#include "debug_utils.h"
#include "node.h"
#include "exceptions.h"
#include "input_manager.h"

// #define PRINT_INIT
// #define PRINT_FINALIZE

namespace elfin {

/* free */
bool is_hub(ModuleType const type) {
    return type == ModuleType::ASYM_HUB or type == ModuleType::SYM_HUB;
}

Transform get_tx(JSON const& xdb_json,
                 size_t const tx_id)
{
    TRACE(tx_id >= xdb_json["n_to_c_tx"].size(),
          ("tx_id > xdb_json[\"n_to_c_tx\"].size()\n"
           "  Either xdb.json is corrupted or "
           "there is an error in dbgen.py.\n"));

    return Transform(xdb_json["n_to_c_tx"][tx_id]);
}

/* public */
/* ctors */
ProtoModule::ProtoModule(std::string const& _name,
                         ModuleType const _type,
                         float const _radius,
                         StrList const& _chain_names) :
    name(_name),
    type(_type),
    radius(_radius) {
#ifdef PRINT_INIT
    JUtil.warn("New ProtoModule at %p\n", this);
#endif  /* ifdef PRINT_INIT */

    for (std::string const& cn : _chain_names) {
        chains_.emplace_back(/*chain_name=*/cn, /*chain_id=*/chains_.size());
#ifdef PRINT_INIT
        Chain& actual = chains_.back();
        JUtil.warn("Created chain[%s] chains_.size()=%zu at %p; "
                   "actual: %p, %p, %p, %p\n",
                   cn.c_str(),
                   chains_.size(),
                   &actual,
                   &actual.c_term_,
                   &actual.c_term_.links(),
                   &actual.n_term_,
                   &actual.n_term_.links());
#endif  /* ifdef PRINT_INIT */
    }

#ifdef PRINT_INIT
    JUtil.warn("First chain: %p ? %p\n", &chains_.at(0), &(chains_[0]));
#endif  /* ifdef PRINT_INIT */
}

/* accessors */
size_t ProtoModule::get_chain_id(std::string const& chain_name) const
{
    auto chain_itr = std::find_if(begin(chains_),
                                  end(chains_),
    [&](auto const & chain) { return chain.name == chain_name; });

    if (chain_itr == end(chains_)) {
        // Verbose diagnostics.
        JUtil.error("Could not find chain named %s in ProtoModule %s\n",
                    chain_name.c_str(), name.c_str());
        JUtil.error("The following chains are present:\n");
        for (auto& chain : chains_) {
            JUtil.error("%s", chain.name.c_str());
        }

        TRACE_NOMSG("Chain Not Found");
        throw ExitException(1, "Chain Not Found");  // Suppress warning.
    }
    else {
        return chain_itr->id;
    }
}

ProtoLink const* ProtoModule::find_link_to(size_t const src_chain_id,
        TermType const src_term,
        PtModKey const dst_mod,
        size_t const dst_chain_id) const
{
    ProtoTerm const& proto_term =
        chains_.at(src_chain_id).get_term(src_term);
    return proto_term.find_link_to(dst_mod, dst_chain_id, opposite_term(src_term));
}

PtPaths ProtoModule::find_paths(FreeTerms const src_terms,
                                PtModKey const dst_mod,
                                FreeTerms const& dst_terms) const
{
    PtPaths res;

    // No need to do DFS if no src or dst terms are free.
    if (src_terms.empty() or dst_terms.empty()) return res;

    PtTermKeySet src_ptterm_set;
    for (auto const& ft : src_terms) {
        src_ptterm_set.insert(&chains_.at(ft.chain_id).get_term(ft.term));
    }

    PtTermKeySet dst_ptterm_set;
    for (auto const& ft : dst_terms) {
        dst_ptterm_set.insert(&dst_mod->chains_.at(ft.chain_id).get_term(ft.term));
    }

    PtTermKeySet visited_ptterms;
    std::unordered_set<PtModKey> visited_mods;
    ProtoPath path(this);

    // [prev_mod]:out_key >>......<links>......>> in_key:[curr_mod]:next_out_key
#define DFS_PARAM PtTermKey const out_key
    std::function<void(DFS_PARAM)> find_paths_dfs;
    find_paths_dfs = [&](DFS_PARAM) {
        visited_ptterms.insert(out_key);

        for (auto const& link : out_key->links()) {
            auto const curr_mod = link->module;
            auto const in_key = &link->get_term();

            // Skip visited_ptterms ptterms.
            if (visited_ptterms.find(in_key) != end(visited_ptterms)) continue;
            visited_ptterms.insert(in_key);

            if (visited_mods.find(curr_mod) != end(visited_mods)) continue;
            visited_mods.insert(curr_mod);

            path.links.push_back(link.get());

            if (curr_mod == dst_mod) {
                // Add path only if in_key is permitted dst set.
                if (dst_ptterm_set.find(in_key) != end(dst_ptterm_set)) {
                    res.push_back(path);
                }
            }

            if (curr_mod != this or
                    src_ptterm_set.find(in_key) != end(src_ptterm_set)) {
                // If curr_mod is src mod, then in_key must be in the
                // permitted src set.

                for (auto const& chain : curr_mod->chains()) {
                    auto visit = [&](PtTermKey const next_out_key) {
                        if (visited_ptterms.find(next_out_key) != end(visited_ptterms)) return;
                        find_paths_dfs(next_out_key);
                    };

                    visit(&chain.n_term());
                    visit(&chain.c_term());
                }
            }

            path.links.pop_back();
            visited_ptterms.erase(in_key);
            visited_mods.erase(curr_mod);
        }

        visited_ptterms.erase(out_key);
    };
#undef DFS_PARAM

    for (auto const ptterm : src_ptterm_set) {
        visited_ptterms.clear();
        visited_mods.clear();
        find_paths_dfs(ptterm);
    }

    return res;
}

/* modifiers */
void ProtoModule::finalize() {
    // ProtoChain finalize() relies on Terminus finalize(), which assumes that
    // all ProtoModule counts are calculated.
    TRACE_NOMSG(already_finalized_);
    already_finalized_ = true;

#ifdef PRINT_FINALIZE
    JUtil.warn("Finalizing module %s\n", name.c_str());
#endif  /* ifdef PRINT_FINALIZE */

    for (ProtoChain& proto_chain : chains_) {
        proto_chain.finalize();

        if (not proto_chain.n_term().links().empty()) {
            free_terms_.emplace_back(
                nullptr,
                proto_chain.id,
                TermType::N);
        }

        if (not proto_chain.c_term().links().empty()) {
            free_terms_.emplace_back(
                nullptr,
                proto_chain.id,
                TermType::C);
        }
    }
}

// Creates links for appropriate chains in both mod_a and mod_b.
//
// Important: mod_a's C-terminus connects to mod_b's N-terminus
// (static)
void ProtoModule::create_proto_link_pair(JSON const& xdb_json,
        size_t const tx_id,
        ProtoModule& mod_a,
        std::string const& a_chain_name,
        ProtoModule& mod_b,
        std::string const& b_chain_name)
{
    // Find A chains.
    PtChains& a_chains = mod_a.chains_;
    size_t const a_chain_id = mod_a.get_chain_id(a_chain_name);
    ProtoChain& a_chain = a_chains.at(a_chain_id);

    // Find B chains.
    PtChains& b_chains = mod_b.chains_;
    size_t const b_chain_id = mod_b.get_chain_id(b_chain_name);
    ProtoChain& b_chain = b_chains.at(b_chain_id);

    // Resolve transformation matrix: C-term extrusion style.
    Transform tx = get_tx(xdb_json, tx_id);

    if (mod_a.type == ModuleType::SINGLE and
            mod_b.type == ModuleType::SINGLE) {
        // Do nothing - Single-to-Single transform is taken care of by
        // dbgen.py.
    }
    else if (mod_a.type == ModuleType::SINGLE and
             mod_b.is_hub()) {
        // Invert tx because dbgen.py only outputs n_to_c_tx for
        // Hub-to-Single.
        tx = tx.inversed();
    }
    else if (mod_a.is_hub() and
             mod_b.type == ModuleType::SINGLE) {
        // Do nothing - Single-to-Single transform is taken care of by
        // dbgen.py.
    }
    else {
        TRACE("Bad mod type combination",
              "mod_a.type == %s and mod_b.type == %s\n",
              ModuleTypeToCStr(mod_a.type),
              ModuleTypeToCStr(mod_b.type));
    }

    // Connect the link duo's reverse pointers.
    {
        auto a_link_sp = std::make_unique<ProtoLink>(tx, &mod_b, b_chain_id, TermType::N);
        auto b_link_sp = std::make_unique<ProtoLink>(tx.inversed(), &mod_a, a_chain_id, TermType::C);

        a_link_sp->reverse = b_link_sp.get();
        b_link_sp->reverse = a_link_sp.get();

        // Create unique_ptr at specific location.
        a_chain.c_term_.links_.push_back(std::move(a_link_sp));
        b_chain.n_term_.links_.push_back(std::move(b_link_sp));
    }

    mod_a.counts_.c_links++;
    if (a_chain.c_term_.links_.size() == 1) { // 0 -> 1 indicates a new interface
        mod_a.counts_.c_interfaces++;
    }

    mod_b.counts_.n_links++;
    if (b_chain.n_term_.links_.size() == 1) { // 0 -> 1 indicates a new interface
        mod_b.counts_.n_interfaces++;
    }
}

/* printers */
void ProtoModule::print_to(std::ostream& os) const {
    os << "ProtoModule(" << name << ":";
    os << (void*) this << ")[" << '\n';
    os << "  Type: " << ModuleTypeToCStr(type) << '\n';
    os << "  Radius: " << radius << '\n';
    os << "  n_link_count: " << counts().n_links << '\n';
    os << "  c_link_count: " << counts().c_links << '\n';
    os << "]";
}

}  /* elfin */