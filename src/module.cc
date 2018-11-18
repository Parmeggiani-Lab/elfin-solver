#include "module.h"

namespace elfin {

// Point3f parse_com_b_from_double(const JSON & j) {
//     std::vector<float> com_bv;
//     for (auto com_bf = j["com_b"].begin();
//             com_bf != j["com_b"].end();
//             ++com_bf) {
//         com_bv.push_back((*com_bf).get<float>());
//     }

//     return Point3f(com_bv);
// }

Mat3x3 parse_rot_from_double(const JSON & j) {
    std::vector<float> rotv;
    for (auto rot_row = j["rot"].begin();
            rot_row != j["rot"].end();
            ++rot_row) {
        for (auto rot_f = (*rot_row).begin();
                rot_f != (*rot_row).end();
                ++rot_f) {
            rotv.push_back((*rot_f).get<float>());
        }
    }

    return Mat3x3(rotv);
}

Vector3f parse_tran_from_double(const JSON & j) {
    std::vector<float> tranv;
    for (auto tran_row = j["tran"].begin();
            tran_row != j["tran"].end();
            ++tran_row) {
        for (auto tran_f = (*tran_row).begin();
                tran_f != (*tran_row).end();
                ++tran_f) {
            tranv.push_back((*tran_f).get<float>());
        }
    }

    return Vector3f(tranv);
}

//static
void link_modules(
    const JSON & tx_json,
    Module * amod,
    const size_t amod_chain_id
    Module * bmod,
    const size_t bmod_chain_id) {

    /*
     * Creates TxMod for appropriate chains in both amod and bmod (transform
     * for bmod is inverted).
     *
     * amod's C-terminus connects to bmod's N-terminus
     */

    if(amod_chain_id >= amod.chains_.size()) {
        die("amod_chain_id >= amod.chains_size()\n");
    }

    if(bmod_chain_id >= bmod.chains_.size()) {
        die("bmod_chain_id >= bmod.chains_size()\n");
    }

    Transform tx = { trans, rot };
    Transform tx_inv = { -trans, rot.transpose() };

    TxMod tm = std::make_tuple(tx, bmod);
    TxMod tm_inv = std::make_tuple(tx_inv, amod);

    amod.chains_[amod_chain_id].c_links.push_back(tm);
    bmod.chains_[bmod_chain_id].n_links.push_back(tm_inv);

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