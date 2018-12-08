#include "proto_module.h"

#include <sstream>

#include "debug_utils.h"

// #define PRINT_INIT
// #define PRINT_CREATE_PROTO_LINK
// #define PRINT_FINALIZE

namespace elfin {

ProtoModule::ProtoModule(const std::string & _name,
                         const ModuleType _type,
                         const float _radius,
                         const StrList & _chain_names) :
    name(_name),
    type(_type),
    radius(_radius),
    chain_names(_chain_names) {
#ifdef PRINT_INIT
    wrn("New ProtoModule at %p\n", this);
#endif  /* ifdef PRINT_INIT */

    for (const std::string & cn : chain_names) {
        chain_id_map_[cn] = chains_.size();
        chains_.emplace_back(cn);
#ifdef PRINT_INIT
        Chain & actual = chains_.back();
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

void ProtoModule::finalize() {
    // ProtoChain finalize() relies on Terminus finalize(), which assumes that
    // all ProtoModule counts are calculated
    NICE_PANIC(finalized_,
               string_format("%s called more than once!", __PRETTY_FUNCTION__).c_str());
    finalized_ = true;

#ifdef PRINT_FINALIZE
    wrn("Finalizing module %s\n", name.c_str());
#endif  /* ifdef PRINT_FINALIZE */

    for (ProtoChain & proto_chain : chains_) {
        proto_chain.finalize();
    }
}

std::string ProtoModule::to_string() const {
    std::ostringstream ss;

    ss << "ProtoModule[" << std::endl;
    ss << "\tName: " << name << std::endl;
    ss << "\tType: " << ModuleTypeNames[type] << std::endl;
    ss << "\tRadius: " << radius << std::endl;
    ss << "\tn_link_count: " << counts().n_links << std::endl;
    ss << "\tc_link_count: " << counts().c_links << std::endl;
    ss << "  ]" << std::endl;

    return ss.str();
}

/*
 * Creates links for appropriate chains in both mod_a and mod_b (transform
 * for mod_b is inverted).
 *
 * mod_a's C-terminus connects to mod_b's N-terminus
 * (static)
 */
void ProtoModule::create_proto_link(
    const JSON & tx_json,
    ProtoModule * mod_a,
    const std::string & a_chain_name,
    ProtoModule * mod_b,
    const std::string & b_chain_name) {
    // Create transforms
    const Transform tx(tx_json);
    const Transform tx_inv = tx.inversed();

    // Find chains
    ProtoChainList & a_chains = mod_a->chains_;
    const size_t a_chain_id = mod_a->chain_id_map().at(a_chain_name);
    ProtoChain & a_chain = a_chains.at(a_chain_id);

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

    ProtoChainList & b_chains = mod_b->chains_;
    const size_t b_chain_id = mod_b->chain_id_map().at(b_chain_name);
    ProtoChain & b_chain = b_chains.at(b_chain_id);

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
    a_chain.c_term_.proto_links_.emplace_back(tx, mod_b, b_chain_id);
    mod_a->counts_.c_links++;
    if (a_chain.c_term_.proto_links().size() == 1) { // 0 -> 1 indicates a new interface
        mod_a->counts_.c_interfaces++;
    }
    b_chain.n_term_.proto_links_.emplace_back(tx_inv, mod_a, a_chain_id);
    mod_b->counts_.n_links++;
    if (b_chain.n_term_.proto_links().size() == 1) { // 0 -> 1 indicates a new interface
        mod_b->counts_.n_interfaces++;
    }
}

}  /* elfin */