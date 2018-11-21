#include "node_list.h"

#include "database.h"

namespace elfin {

/* static data members */
size_t NodeList::MAX_LEN_ = 0;
const size_t & NodeList::MAX_LEN = NodeList::MAX_LEN_;

/*
 * Checks whether new_com is too close to any other com.
 */
bool NodeList::collides(
    const Vector3f & new_com,
    const float mod_radius) const {

    for (const auto & node : *this) {
        const float sq_com_dist = node.tx.collapsed().sq_dist_to(new_com);
        const float required_com_dist = mod_radius +
                                        node.prototype->radius_;
        if (sq_com_dist < (required_com_dist * required_com_dist))
            return true;
    }

    return false;
}

/*
 * Finds points at which a severance would allow a different limb to be grown.
 * (Modifies "id_out")
 * (static)
 */
bool NodeList::pick_sever_point(
    const NodeList & nodes,
    size_t & id_out,
    const TerminusType term) {

    bool success = false;
    std::vector<size_t> non_dead_end_ids;

    for (size_t i = 0; i < nodes.size(); ++i) {
        const Module * proto = nodes.at(i).prototype;
        if ((term == N and proto->n_link_count() > 1) or
                (term == C and proto->c_link_count() > 1) or
                (term == ANY and proto->all_link_count() > 2)) {

            non_dead_end_ids.push_back(i);

        }
    }

    if (!non_dead_end_ids.empty()) {
        id_out = pick_random(non_dead_end_ids);
        success = true;
    }

    return success;
}

/*
 * Grows a random limb of length "n_nodes" starting from "head" at "term"
 * terminus.
 */
void NodeList::randomize(
    NodeList::const_iterator head,
    const TerminusType term,
    const size_t n_nodes) {

    die("Not rewritten\n");

    // Cut limb

    // if (nodes.empty()) {
    //     // Pick random starting node
    //     nodes.emplace_back(XDB.get_rand_mod(), Transform());
    // }

    // while (nodes.size() <= max_len) {
    //     // Find list of non-colliding neighbour modules
    //     std::vector<const Module::Link *> compat_links;
    //     const Transform & base_tx = nodes.back().tx;
    //     const Module * node_proto = nodes.back().prototype;

    //     Roulette roulette;
    //     size_t total_c_links = 0;
    //     for (auto & chain_map_it : node_proto->chains()) {
    //         const Module::Chain & chain = chain_map_it.second;
    //         for (auto & c_link : chain.c_links) {
    //             Transform tx = c_link.tx * base_tx;

    //             if (!nodes.collides(
    //                         tx.collapsed(),
    //                         node_proto->radius_)) {
    //                 compat_links.push_back(&c_link);
    //                 const float clc = node_proto->c_link_count();
    //                 roulette.cmlprobs.push_back(total_c_links);
    //                 total_c_links += clc;
    //             }
    //         }
    //     }

    //     if (compat_links.empty())
    //         break;

    //     roulette.normalize(total_c_links);

    //     // Pick a random valid neighbour
    //     const Module::Link * link = roulette.rand_item(compat_links);

    //     // Grow shape
    //     nodes.emplace_back(link->mod, link->tx * base_tx);
    // }
}

}  /* elfin */