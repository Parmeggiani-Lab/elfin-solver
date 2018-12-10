#include "proto_module.h"

#include <sstream>

#include "debug_utils.h"

// #define PRINT_INIT
// #define PRINT_CREATE_PROTO_LINK
// #define PRINT_FINALIZE

namespace elfin {

/* ctors */
ProtoModule::ProtoModule(const std::string & _name,
                         const ModuleType _type,
                         const float _radius,
                         const StrList & _chain_names) :
    name(_name),
    type(_type),
    radius(_radius) {
#ifdef PRINT_INIT
    wrn("New ProtoModule at %p\n", this);
#endif  /* ifdef PRINT_INIT */

    for (const std::string & cn : _chain_names) {
        chains_.emplace_back(cn, chains_.size());
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

/* accessors */
size_t ProtoModule::find_chain_id(const std::string & chain_name) const {
    for (auto & chain : chains_) {
        if (chain.name == chain_name) {
            return chain.id;
        }
    }

    err("Could not find chain named %s in ProtoModule %s\n",
        chain_name, chain_name.c_str());

    err("The following chains are present:\n");
    for (auto & chain : chains_) {
        err("%s", chain.name.c_str());
    }

    NICE_PANIC("Chain Not Found");
}

Vector<const ProtoLink *> ProtoModule::find_all_links_to(
    const TerminusType src_term,
    const ProtoModule * src_module,
    const size_t dst_chain_id) const {
    Vector<const ProtoLink *> res;

    // Collect links from each chain
    for (size_t i = 0; i < chains_.size(); ++i) {
        const ProtoLink * link_ptr =
            find_link_to(i, src_term, src_module, dst_chain_id);
        if (link_ptr) {
            res.push_back(link_ptr);
        }
    }

    return res;
}

const ProtoLink * ProtoModule::find_link_to(
    const size_t src_chain_id,
    const TerminusType src_term,
    const ProtoModule * dst_module,
    const size_t dst_chain_id) const {

    const ProtoTerminus & proto_term =
        chains_.at(src_chain_id).get_term(src_term);
    ProtoLinkPtrSetCItr itr =
        proto_term.find_link_to(dst_module, dst_chain_id);

    if (itr != proto_term.proto_link_set().end()) {
        return *itr;
    }

    return nullptr;
}

bool ProtoModule::has_link_to(
    const TerminusType src_term,
    ConstProtoModulePtr module,
    const size_t chain_id) const {
    // Return true on first find
    for (const ProtoChain & chain : chains_) {
        const ProtoTerminus & proto_term =
            chain.get_term(src_term);
        ProtoLinkPtrSetCItr itr =
            proto_term.find_link_to(module, chain_id);
        if (itr != proto_term.proto_link_set().end()) {
            return true;
        }
    }

    return false;
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

    for (ProtoChain & proto_chain : chains_) {
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
    const size_t a_chain_id = mod_a->find_chain_id(a_chain_name);
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
        &a_chain.c_term_.proto_link_list_,
        &a_chain.n_term_,
        &a_chain.n_term_.proto_link_list_);
#endif  /* ifdef PRINT_CREATE_PROTO_LINK */

    ProtoChainList & b_chains = mod_b->chains_;
    const size_t b_chain_id = mod_b->find_chain_id(b_chain_name);
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
        &b_chain.c_term_.proto_link_list_,
        &b_chain.n_term_,
        &b_chain.n_term_.proto_link_list_);
#endif  /* ifdef PRINT_CREATE_PROTO_LINK */

    // Create links and count
    a_chain.c_term_.proto_link_list_.emplace_back(tx, mod_b, b_chain_id);
    mod_a->counts_.c_links++;
    if (a_chain.c_term_.proto_links().size() == 1) { // 0 -> 1 indicates a new interface
        mod_a->counts_.c_interfaces++;
    }
    b_chain.n_term_.proto_link_list_.emplace_back(tx_inv, mod_a, a_chain_id);
    mod_b->counts_.n_links++;
    if (b_chain.n_term_.proto_links().size() == 1) { // 0 -> 1 indicates a new interface
        mod_b->counts_.n_interfaces++;
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