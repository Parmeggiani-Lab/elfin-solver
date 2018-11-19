#include "module.h"

namespace elfin {

//static
void link_chains(
    const JSON & tx_json,
    Chain & a_chain,
    Chain & b_chain) {
    /*
     * Creates TxMod for appropriate chains in both amod and bmod (transform
     * for bmod is inverted).
     *
     * amod's C-terminus connects to bmod's N-terminus
     */

    Transform tx(tx_json);
    Transform tx_inv = tx.inversed();

    TxMod tm = std::make_tuple(tx, bmod);
    TxMod tm_inv = std::make_tuple(tx_inv, amod);

    a_chain.c_links.push_back(tm);
    b_chain.n_links.push_back(tm_inv);

    amod.c_link_count++;
    bmod.n_link_count++;
}

std::string Module::to_string() const
{
    std::ostringstream ss;

    ss << "Mod[" << std::endl;
    ss << "\tcom_b:" << com_b.to_string() << std::endl;
    ss << "\trot:" << rot.to_string() << std::endl;
    ss << "\trot_inv:" << rot_inv.to_string() << std::endl;
    ss << "\ttran:" << tran.to_string() << std::endl;
    ss << "  ]" << std::endl;

    return ss.str();
}

}  /* elfin */