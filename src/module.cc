#include "module.h"

#include <sstream>

namespace elfin {

Module::Module(const std::string & _name,
               const ModuleType _type,
               const float _radius,
               const StrList & _chain_names) :
    name(_name),
    type(_type),
    radius(_radius),
    chain_names(_chain_names) {
    wrn("New Module at 0x%x\n", this);
    size_t i = 0;

    // chains_.reserve(chain_names.size());
    for (const std::string & cn : chain_names) {
        chain_id_map_[cn] = chains_.size();
        chains_.emplace_back(cn);
        Chain & actual = chains_.back();
        wrn("Created chain[%s] #%lu, chains_.size()=%lu at 0x%x ? 0x%x; actual: 0x%x, 0x%x, 0x%x, 0x%x\n",
            cn.c_str(),
            i,
            chains_.size(),
            &actual,
            &chains_.at(i),
            &actual.c_term_,
            &actual.c_term_.links,
            &actual.n_term_,
            &actual.n_term_.links);
        i++;
    }
    wrn("First chain: 0x%x ? 0x%x\n", &chains_.at(0), &(chains_[0]));
}

void Module::finalize() {
    // Chain finalize() relies on Terminus finalize(), which assumes that
    // all Module counts are calculated
    for (Chain & chain : chains_) {
        chain.finalize();
    }
}

std::string Module::to_string() const {
    std::ostringstream ss;

    ss << "Module[" << std::endl;
    ss << "\tName: " << name << std::endl;
    ss << "\tType: " << ModuleTypeNames[type] << std::endl;
    ss << "\tRadius: " << radius << std::endl;
    ss << "\tn_link_count: " << counts.n_link << std::endl;
    ss << "\tc_link_count: " << counts.c_link << std::endl;
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
void Module::create_link(
    const JSON & tx_json,
    Module * mod_a,
    const std::string & a_chain_name,
    Module * mod_b,
    const std::string & b_chain_name) {
    // Create transforms
    const Transform tx(tx_json);
    const Transform tx_inv = tx.inversed();

    // Find chains
    ChainList & a_chains = mod_a->chains_;
    const size_t a_chain_id = mod_a->chain_id_map.at(a_chain_name);
    Chain & a_chain = a_chains.at(a_chain_id);
    wrn("mod_a[0x%x:%s] size: %lu, chain[0x%x:%s:%lu], counts: %lu, %lu, %lu\n",
        mod_a,
        mod_a->name.c_str(),
        a_chains.size(),
        &a_chain,
        a_chain.name.c_str(),
        a_chain_id,
        mod_a->counts.n_link,
        mod_a->counts.c_link,
        mod_a->counts.interface);
    wrn("a_chain: 0x%x, 0x%x, 0x%x, 0x%x\n",
        &a_chain.c_term_,
        &a_chain.c_term_.links,
        &a_chain.n_term_,
        &a_chain.n_term_.links);

    ChainList & b_chains = mod_b->chains_;
    const size_t b_chain_id = mod_b->chain_id_map.at(b_chain_name);
    Chain & b_chain = b_chains.at(b_chain_id);
    wrn("mod_b[0x%x:%s], size: %lu, chain[0x%x:%s:%lu], counts: %lu, %lu, %lu\n",
        mod_b,
        mod_b->name.c_str(),
        b_chains.size(),
        &b_chain,
        b_chain.name.c_str(),
        b_chain_id,
        mod_b->counts.n_link,
        mod_b->counts.c_link,
        mod_b->counts.interface);
    wrn("b_chain: 0x%x, 0x%x, 0x%x, 0x%x\n",
        &b_chain.c_term_,
        &b_chain.c_term_.links,
        &b_chain.n_term_,
        &b_chain.n_term_.links);

    // Create links and count
    a_chain.c_term_.links_.emplace_back(tx, mod_b, b_chain_id);
    mod_a->counts_.c_link++;
    if (a_chain.c_term_.links.size() == 1) { // 0 -> 1 indicates a new interface
        mod_a->counts_.interface++;
    }
    b_chain.n_term_.links_.emplace_back(tx_inv, mod_a, a_chain_id);
    mod_b->counts_.n_link++;
    if (b_chain.n_term_.links.size() == 1) { // 0 -> 1 indicates a new interface
        mod_b->counts_.interface++;
    }
}

}  /* elfin */