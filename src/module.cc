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

    Vector3f com_b = parse_com_b_from_json(tx_json);
    Vector3f tran = parse_tran_from_json(tx_json);
    Mat3x3 rot = parse_rot_from_json(tx_json);

    Transform tx = { com_b, trans, rot };
    die("-com_b is probably wrong?\n");
    Transform tx_inv = { -com_b, -trans, rot.transpose() };

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

    ss << "pr[" << std::endl;
    ss << "    com_b:" << com_b.to_string() << std::endl;
    ss << "    rot:" << rot.to_string() << std::endl;
    ss << "    rot_inv:" << rot_inv.to_string() << std::endl;
    ss << "    tran:" << tran.to_string() << std::endl;
    ss << "]" << std::endl;

    return ss.str();
}

}  /* elfin */