#include "proto_module.h"

#include <sstream>

#include "debug_utils.h"
#include "node.h"

// #define PRINT_INIT
// #define PRINT_CREATE_PROTO_LINK
// #define PRINT_FINALIZE

namespace elfin {

/* ctors */
ProtoModule::ProtoModule(
    std::string const& _name,
    ModuleType const _type,
    float const _radius,
    StrList const& _chain_names) :
    name(_name),
    type(_type),
    radius(_radius) {
#ifdef PRINT_INIT
    wrn("New ProtoModule at %p\n", this);
#endif  /* ifdef PRINT_INIT */

    for (std::string const& cn : _chain_names) {
        chains_.emplace_back(cn, chains_.size());
#ifdef PRINT_INIT
        Chain& actual = chains_.back();
        wrn("Created chain[%s] chains_.size()=%lu at %p; actual: %p, %p, %p, %p\n",
            cn.c_str(),
            chains_.size(),
            &actual,
            &actual.c_term_,
            &actual.c_term_.proto_links(),
            &actual.n_term_,
            &actual.n_term_.proto_links());
#endif  /* ifdef PRINT_INIT */
    }

#ifdef PRINT_INIT
    wrn("First chain: %p ? %p\n", &chains_.at(0), &(chains_[0]));
#endif  /* ifdef PRINT_INIT */
}

/* accessors */
size_t ProtoModule::find_chain_id(
    std::string const& chain_name) const {
    for (auto& chain : chains_) {
        if (chain.name == chain_name) {
            return chain.id;
        }
    }

    err("Could not find chain named %s in ProtoModule %s\n",
        chain_name, chain_name.c_str());

    err("The following chains are present:\n");
    for (auto& chain : chains_) {
        err("%s", chain.name.c_str());
    }

    NICE_PANIC("Chain Not Found");
}
ProtoLink const* ProtoModule::find_link_to(
    size_t const src_chain_id,
    TerminusType const src_term,
    ProtoModule const* dst_module,
    size_t const dst_chain_id) const {

    ProtoTerminus const& proto_term =
        chains_.at(src_chain_id).get_term(src_term);
    ProtoLinkPtrSetCItr itr =
        proto_term.find_link_to(dst_module, dst_chain_id);

    if (itr != proto_term.proto_link_set().end()) {
        return *itr;
    }

    return nullptr;
}

ProtoModule::BridgeList ProtoModule::find_bridges(
    Link const& arrow) const {
    BridgeList res;

    FreeChain const& src = arrow.src();
    FreeChain const& dst = arrow.dst();

    DEBUG(src.node->prototype() != this);
    ProtoModule const* dst_mod = dst.node->prototype();
    const size_t dst_chain_id = dst.chain_id;

    ProtoTerminus const& ptterm_src =
        chains_.at(src.chain_id).get_term(src.term);

    // For each middle ProtoModule that ptterm_src connects to...
    for (ProtoLink const* ptlink1 : ptterm_src.proto_links()) {
        ProtoModule const* const middle_mod = ptlink1->module();

        // Skip non basic modules
        if (middle_mod->counts().all_interfaces() > 2) {
            continue;
        }

        size_t const chain_in = ptlink1->chain_id();

        // Look for ptlink2 to dst_mod
        for (ProtoChain const& middle_chain : middle_mod->chains_) {
            // Skip incoming chain.
            if (middle_chain.id == src.chain_id) continue;

            // dst.term is incoming term, the opposite of which is src.term.
            ProtoTerminus const& ptterm_out =
                middle_chain.get_term(src.term);
            ProtoLinkPtrSetCItr itr =
                ptterm_out.find_link_to(dst_mod, dst_chain_id);

            if (itr != ptterm_out.proto_link_set().end()) {
                // We have found a ptlink2
                res.emplace_back(ptlink1, *itr);
            }
        }
    }

    return res;
}

/* modifiers */
void ProtoModule::finalize() {
    // ProtoChain finalize() relies on Terminus finalize(), which assumes that
    // all ProtoModule counts are calculated
    NICE_PANIC(finalized_,
               string_format("%s called more than once!", __PRETTY_FUNCTION__).c_str());
    finalized_ = true;

#ifdef PRINT_FINALIZE
    wrn("Finalizing module %s\n", name.c_str());
#endif  /* ifdef PRINT_FINALIZE */

    for (ProtoChain& proto_chain : chains_) {
        proto_chain.finalize();
    }
}

/*
 * Creates links for appropriate chains in both mod_a and mod_b (transform
 * for mod_b is inverted).
 *
 * mod_a's C-terminus connects to mod_b's N-terminus
 * (static)
 */
void ProtoModule::create_proto_link_pair(
    JSON const& tx_json,
    ProtoModule* mod_a,
    std::string const& a_chain_name,
    ProtoModule* mod_b,
    std::string const& b_chain_name) {
    // Create transforms
    Transform const tx(tx_json);
    Transform const tx_inv = tx.inversed();

    // Find chains
    ProtoChainList& a_chains = mod_a->chains_;
    size_t const a_chain_id = mod_a->find_chain_id(a_chain_name);
    ProtoChain& a_chain = a_chains.at(a_chain_id);

#ifdef PRINT_CREATE_PROTO_LINK
    wrn(("mod_a[%p:%s] size: %lu, proto_chain[%p:%s:%lu], "
         "counts: links(%lu, %lu), interface(%lu, %lu)\n"),
        mod_a,
        mod_a->name.c_str(),
        a_chains.size(),
        &a_chain,
        a_chain.name.c_str(),
        a_chain_id,
        mod_a->counts().n_link,
        mod_a->counts().c_link,
        mod_a->counts().n_interfaces,
        mod_a->counts().c_interfaces);
    wrn("a_chain: %p, %p, %p, %p\n",
        &a_chain.c_term_,
        &a_chain.c_term_.proto_links_,
        &a_chain.n_term_,
        &a_chain.n_term_.proto_links_);
#endif  /* ifdef PRINT_CREATE_PROTO_LINK */

    ProtoChainList& b_chains = mod_b->chains_;
    size_t const b_chain_id = mod_b->find_chain_id(b_chain_name);
    ProtoChain& b_chain = b_chains.at(b_chain_id);

#ifdef PRINT_CREATE_PROTO_LINK
    wrn("mod_b[%p:%s] size: %lu, proto_chain[%p:%s:%lu], "
        "counts: links(%lu, %lu), interface(%lu, %lu)\n",
        mod_b,
        mod_b->name.c_str(),
        b_chains.size(),
        &b_chain,
        b_chain.name.c_str(),
        b_chain_id,
        mod_b->counts().n_link,
        mod_b->counts().c_link,
        mod_b->counts().n_interfaces,
        mod_b->counts().c_interfaces);
    wrn("b_chain: %p, %p, %p, %p\n",
        &b_chain.c_term_,
        &b_chain.c_term_.proto_links_,
        &b_chain.n_term_,
        &b_chain.n_term_.proto_links_);
#endif  /* ifdef PRINT_CREATE_PROTO_LINK */

    // Create links and count
    ProtoLink* a_ptlink = new ProtoLink(tx, mod_b, b_chain_id);
    a_chain.c_term_.proto_links_.push_back(a_ptlink);
    mod_a->counts_.c_links++;
    if (a_chain.c_term_.proto_links_.size() == 1) { // 0 -> 1 indicates a new interface
        mod_a->counts_.c_interfaces++;
    }

    ProtoLink* b_ptlink = new ProtoLink(tx_inv, mod_a, a_chain_id);
    b_chain.n_term_.proto_links_.push_back(b_ptlink);
    mod_b->counts_.n_links++;
    if (b_chain.n_term_.proto_links_.size() == 1) { // 0 -> 1 indicates a new interface
        mod_b->counts_.n_interfaces++;
    }

    ProtoLink::pair_proto_links(a_ptlink, b_ptlink);
}

/* printers */
std::string ProtoModule::to_string() const {
    std::ostringstream ss;

    ss << "ProtoModule[" << std::endl;
    ss << "\tName: " << name << std::endl;
    ss << "\tType: " << ModuleTypeToCStr(type) << std::endl;
    ss << "\tRadius: " << radius << std::endl;
    ss << "\tn_link_count: " << counts().n_links << std::endl;
    ss << "\tc_link_count: " << counts().c_links << std::endl;
    ss << "  ]" << std::endl;

    return ss.str();
}

}  /* elfin */