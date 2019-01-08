#include "proto_module.h"

#include <sstream>
#include <memory>

#include "debug_utils.h"
#include "node.h"
#include "exceptions.h"

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
    return proto_term.find_link_to(dst_mod, dst_chain_id);
}

// bool has_path_to(PtModKey const target_mod,
//                  PtModKey const prev_mod,
//                  PtModVisitMap& visited)
// {
//     DEBUG_NOMSG(not target_mod);
//     DEBUG_NOMSG(not prev_mod);

//     // DFS search for target_mod.
//     visited[prev_mod] = true;

//     auto check_ptterm = [&](ProtoTerm const & ptterm) {
//         return any_of(begin(ptterm.links()), end(ptterm.links()),
//         [&](auto const & ptlink) {
//             auto const dst = ptlink->module_;
//             return dst == target_mod or
//                    (not visited[dst] and has_path_to(target_mod, dst, visited));
//         });
//     };

//     for (auto const& ptchain : prev_mod->chains()) {
//         if (check_ptterm(ptchain.n_term()) or
//                 check_ptterm(ptchain.c_term())) {
//             return true;
//         }
//     }

//     return false;
// }

using PtTermSet = std::unordered_set<PtTermKey>;

void proto_path_dfs(PtModKey const curr_mod,
                    ProtoTerm const& curr_ptterm,
                    ProtoPath&& curr_path,
                    PtModKey const dst_mod,
                    FreeTerms const& dst_fterms,
                    PtPaths& res,
                    PtTermSet& visited)
{
    if (curr_path.links.size() >= 2 and curr_mod == dst_mod) {
        // Terminating condition met.
        res.push_back(curr_path);
    }
    else {
        for (auto const& ptlink : curr_ptterm.links()) {

        }
    }
}

PtPaths ProtoModule::find_paths(FreeTerm const& src_fterm,
                                PtModKey const dst_mod,
                                FreeTerms const& dst_fterms) const
{
    PtPaths res;

    // No need to do DFS if no dst terms are free.
    if (not dst_fterms.empty()) {
        auto const& src_ptterm = chains_.at(src_fterm.chain_id).get_term(src_fterm.term);
        PtTermSet visited = { &src_ptterm };
        proto_path_dfs(this,
                       src_ptterm,
                       ProtoPath(this),
                       dst_mod,
                       dst_fterms,
                       res,
                       visited);
    }

    return res;
}

/* modifiers */
void ProtoModule::finalize() {
    // ProtoChain finalize() relies on Terminus finalize(), which assumes that
    // all ProtoModule counts are calculated
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
                TermType::N,
                proto_chain.id);
        }

        if (not proto_chain.c_term().links().empty()) {
            free_terms_.emplace_back(
                nullptr,
                TermType::C,
                proto_chain.id);
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

    {
        // Allocate raw memory.
        auto a_link_addr = (ProtoLink*) malloc(sizeof(ProtoLink));
        auto b_link_addr = (ProtoLink*) malloc(sizeof(ProtoLink));

        // Create unique_ptr at specific location.
        a_chain.c_term_.links_.emplace_back(
            new(a_link_addr) ProtoLink(tx, &mod_b, b_chain_id, b_link_addr));
        b_chain.n_term_.links_.emplace_back(
            new(b_link_addr) ProtoLink(tx.inversed(), &mod_a, a_chain_id, a_link_addr));
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