#include "module.h"

#include <sstream>

namespace elfin {

//static
void Module::link_chains(
    const JSON & tx_json,
    Module * mod_a,
    const std::string & a_chain_id,
    Module * mod_b,
    const std::string & b_chain_id) {
    /*
     * Creates TxMod for appropriate chains in both amod and bmod (transform
     * for bmod is inverted).
     *
     * amod's C-terminus connects to bmod's N-terminus
     */

    const Transform tx(tx_json);
    const Transform tx_inv = tx.inversed();

    Chain & a_chain = mod_a->chains_[a_chain_id];
    Chain & b_chain = mod_b->chains_[b_chain_id];

    a_chain.c_links.push_back(std::make_tuple(tx, mod_b));
    b_chain.n_links.push_back(std::make_tuple(tx_inv, mod_a));

    mod_a->c_link_count_++;
    mod_b->n_link_count_++;
}

std::string Module::to_string() const
{
    std::ostringstream ss;

    ss << "Mod[" << std::endl;
    ss << "\tradii:" << std::endl;
    ss << "\t\tavg_all=" << radii_.avg_all << std::endl;
    ss << "\t\tmax_ca=" << radii_.max_ca << std::endl;
    ss << "\t\tmax_heavy=" << radii_.max_heavy << std::endl;
    ss << "\tn_link_count:" << n_link_count_ << std::endl;
    ss << "\tc_link_count:" << c_link_count_ << std::endl;
    ss << "  ]" << std::endl;

    return ss.str();
}

}  /* elfin */