#include "module.h"

#include <sstream>

namespace elfin {

Module::Module(const std::string & name,
               const ModuleType type,
               const float radius,
               const StrList & chain_names) :
    name_(name),
    type_(type),
    radius_(radius),
    chain_names_(chain_names) {
    for (auto & cn : chain_names_) {
        chain_id_map_[cn] = chains_.size();
        chains_.emplace_back(cn);
    }
}

void Module::finalize() {
    for (auto & chain : chains_) {
        chain.finalize();

        const size_t cls = chain.c_links.size();
        const size_t nls = chain.n_links.size();

        counts_.c_link += cls;
        counts_.n_link += nls;
        counts_.interface += (cls > 0) + (nls > 0);
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

    ChainList & b_chains = mod_b->chains_;
    const size_t b_chain_id = mod_b->chain_id_map.at(b_chain_name);
    Chain & b_chain = b_chains.at(b_chain_id);

    // Create links
    a_chain.c_links_.emplace_back(tx, mod_b, b_chain_id);
    b_chain.n_links_.emplace_back(tx_inv, mod_a, a_chain_id);
}

}  /* elfin */