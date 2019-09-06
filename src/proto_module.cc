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

namespace elfin {

/* free */
bool is_hub(ModuleType const type) {
    return type == ModuleType::ASYM_HUB or type == ModuleType::SYM_HUB;
}

Transform get_tx(JSON const& xdb_json,
                 size_t const tx_id)
{
    TRACE(tx_id >= xdb_json.at("n_to_c_tx").size(),
          ("tx_id > xdb_json[\"n_to_c_tx\"].size()\n"
           "  Either xdb.json is corrupted or "
           "there is an error in dbgen.py.\n"));

    return Transform(xdb_json.at("n_to_c_tx")[tx_id]);
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
ProtoChain const& ProtoModule::get_chain(size_t const id) const {
    if (id >= chains_.size()) {
        throw OutOfRange("No chain id " + std::to_string(id) +
                         " in " + name);
    }
    else {
        return chains_.at(id);
    }
}

size_t ProtoModule::get_chain_id(std::string const& chain_name) const
{
    auto chain_itr = std::find_if(begin(chains_),
                                  end(chains_),
    [&](auto const & chain) { return chain.name == chain_name; });

    if (chain_itr == end(chains_)) {
        // Verbose diagnostics.
        std::ostringstream oss;
        oss << "Could not find chain named ";
        oss << chain_name;
        oss << " in ProtoModule ";
        oss << name << "\n";

        oss << "The following chains are present:\n";
        for (auto& chain : chains_) {
            oss << chain.name << "\n";
        }

        throw ValueNotFound(oss.str());
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

ProtoTerm const& ProtoModule::get_term(FreeTerm const& ft) const {
    DEBUG_NOMSG(ft.chain_id >= chains_.size());

    return chains_.at(ft.chain_id).get_term(ft.term);
}

PtTermFinderSet ProtoModule::get_reachable_ptterms(FreeTerms const& src_terms) const
{
    PtTermFinderSet res;
    PtTermKeySet visited_out_keys;

    // Keys in frontier are always outward ProtoTerms.
    std::deque<PtTermKey> frontier;
    auto const add_to_frontier = [&visited_out_keys, &frontier](
    PtTermKey const out_key, PtTermKey const in_key) {
        if (out_key != in_key and
                visited_out_keys.find(out_key) == end(visited_out_keys)) {
            frontier.push_back(out_key);
        }
    };

    for (auto const& ft : src_terms) {
        frontier.push_back(&get_term(ft));
    }

    // Don't let src_terms go into visited_out_keys because that is for
    // strictly outward ProtoTerm on another module or another instance of src
    // mod, not the origin src mod.
    size_t skip = src_terms.size();
    while (not frontier.empty()) {
        // Consume front of queue.
        auto const curr_key = frontier.front();
        frontier.pop_front();

        // Put all unvisited outward ProtoTerm on the next wave into frontier.
        for (auto const& link : curr_key->links()) {
            auto const in_key = &link->get_term();

            // const_cast is due to laziness. It should be safe.
            res.insert({link->module, link->chain_id, link->term, in_key});  // Only add inward ProtoTerm to result.
            for (auto const& chain : link->module->chains()) {
                add_to_frontier(&chain.n_term(), in_key);
                add_to_frontier(&chain.c_term(), in_key);
            }
        }

        if (skip > 0) {
            skip--;
        }
        else {
            visited_out_keys.insert(curr_key);
        }
    }

    return res;
}

/* modifiers */
void ProtoModule::configure() {
    // ProtoChain configure() calls Terminus configure(), which assumes that all
    // ProtoModule counts have been calculated.
    free_terms_.clear();

    for (ProtoChain& proto_chain : chains_) {
        proto_chain.configure(name);

        if (not proto_chain.n_term().links().empty() and
                proto_chain.n_term().is_active()) {
            free_terms_.emplace_back(
                nullptr,
                proto_chain.id,
                TermType::N);
        }

        if (not proto_chain.c_term().links().empty() and
                proto_chain.c_term().is_active()) {
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