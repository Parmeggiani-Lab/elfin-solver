#include "module.h"

#include <sstream>

namespace elfin {

/*
 * Creates links for appropriate chains in both mod_a and mod_b (transform
 * for mod_b is inverted).
 *
 * mod_a's C-terminus connects to mod_b's N-terminus
 * (static)
 */
void Module::link_chains(
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
    const size_t a_chain_id = mod_a->chain_id_map().at(a_chain_name);
    Chain & a_chain = a_chains.at(a_chain_id);

    ChainList & b_chains = mod_b->chains_;
    const size_t b_chain_id = mod_b->chain_id_map().at(b_chain_name);
    Chain & b_chain = b_chains.at(b_chain_id);

    // Create links
    a_chain.c_links.emplace_back(tx, mod_b, b_chain_id);
    b_chain.n_links.emplace_back(tx_inv, mod_a, a_chain_id);
}

/*
 * Counts interfaces. Supposed to be called after database gets parsed from
 * JSON.
 */
void Module::finalize() {
    for (auto & chain : chains_) {
        const size_t cls = chain.c_links.size();
        const size_t nls = chain.n_links.size();

        c_link_count_ += cls;
        n_link_count_ += nls;
        interface_count_ += (cls > 0) + (nls > 0);
    }
}

std::string Module::to_string() const
{
    std::ostringstream ss;

    ss << "Mod[" << std::endl;
    ss << "\tName: " << name_ << std::endl;
    ss << "\tType: " << ModuleTypeNames[type_] << std::endl;
    ss << "\tRadius: " << radius_ << std::endl;
    ss << "\tn_link_count: " << n_link_count_ << std::endl;
    ss << "\tc_link_count: " << c_link_count_ << std::endl;
    ss << "  ]" << std::endl;

    return ss.str();
}

//static
Module::CmlSumFunctor Module::n_link_count_functor =
[](Module *& m) {
    return m->n_link_count();
};

//static
Module::CmlSumFunctor Module::c_link_count_functor =
[](Module *& m) {
    return m->c_link_count();
};

//static
Module::CmlSumFunctor Module::all_link_count_functor =
[](Module *& m) {
    return m->all_link_count();
};

}  /* elfin */