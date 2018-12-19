#include "proto_module.h"

#include <sstream>
#include <memory>

#include "debug_utils.h"
#include "node.h"

// #define PRINT_INIT
// #define PRINT_CREATE_PROTO_LINK
// #define PRINT_FINALIZE

namespace elfin {

/* ProtoModule::Counts */

/* ProtoModule */
/* public */
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
            &actual.c_term_.links(),
            &actual.n_term_,
            &actual.n_term_.links());
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
    exit(1); // suppress no return warning
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

    if (itr != proto_term.link_set().end()) {
        return *itr;
    }

    return nullptr;
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
    ProtoModule& mod_a,
    std::string const& a_chain_name,
    ProtoModule& mod_b,
    std::string const& b_chain_name) {
    // Create transforms
    Transform const tx(tx_json);
    Transform const tx_inv = tx.inversed();

    // Find chains
    ProtoChainList& a_chains = mod_a.chains_;
    size_t const a_chain_id = mod_a.find_chain_id(a_chain_name);
    ProtoChain& a_chain = a_chains.at(a_chain_id);

#ifdef PRINT_CREATE_PROTO_LINK
    wrn(("mod_a[%p:%s] size: %lu, proto_chain[%p:%s:%lu], "
         "counts: links(%lu, %lu), interface(%lu, %lu)\n"),
        &mod_a,
        mod_a.name.c_str(),
        a_chains.size(),
        &a_chain,
        a_chain.name.c_str(),
        a_chain_id,
        mod_a.counts().n_link,
        mod_a.counts().c_link,
        mod_a.counts().n_interfaces,
        mod_a.counts().c_interfaces);
    wrn("a_chain: %p, %p, %p, %p\n",
        &a_chain.c_term_,
        &a_chain.c_term_.links_,
        &a_chain.n_term_,
        &a_chain.n_term_.links_);
#endif  /* ifdef PRINT_CREATE_PROTO_LINK */

    ProtoChainList& b_chains = mod_b.chains_;
    size_t const b_chain_id = mod_b.find_chain_id(b_chain_name);
    ProtoChain& b_chain = b_chains.at(b_chain_id);

#ifdef PRINT_CREATE_PROTO_LINK
    wrn("mod_b[%p:%s] size: %lu, proto_chain[%p:%s:%lu], "
        "counts: links(%lu, %lu), interface(%lu, %lu)\n",
        &mod_b,
        mod_b.name.c_str(),
        b_chains.size(),
        &b_chain,
        b_chain.name.c_str(),
        b_chain_id,
        mod_b.counts().n_link,
        mod_b.counts().c_link,
        mod_b.counts().n_interfaces,
        mod_b.counts().c_interfaces);
    wrn("b_chain: %p, %p, %p, %p\n",
        &b_chain.c_term_,
        &b_chain.c_term_.links_,
        &b_chain.n_term_,
        &b_chain.n_term_.links_);
#endif  /* ifdef PRINT_CREATE_PROTO_LINK */

    // Create links and count
    auto a_ptlink = std::make_shared<ProtoLink>(tx_inv, &mod_b, b_chain_id);
    auto b_ptlink = std::make_shared<ProtoLink>(tx, &mod_a, a_chain_id);
    ProtoLink::pair_links(a_ptlink.get(), b_ptlink.get());

    a_chain.c_term_.links_.push_back(a_ptlink);
    mod_a.counts_.c_links++;
    if (a_chain.c_term_.links_.size() == 1) { // 0 -> 1 indicates a new interface
        mod_a.counts_.c_interfaces++;
    }

    b_chain.n_term_.links_.push_back(b_ptlink);
    mod_b.counts_.n_links++;
    if (b_chain.n_term_.links_.size() == 1) { // 0 -> 1 indicates a new interface
        mod_b.counts_.n_interfaces++;
    }
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