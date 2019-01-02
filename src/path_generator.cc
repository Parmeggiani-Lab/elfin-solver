#include "path_generator.h"

#include "link.h"
#include "node.h"
#include "vector3f.h"

namespace elfin {

/* public */
/* ctors */
PathGenerator::PathGenerator(NodeKey start_node) :
    next_node_(start_node)
{
    // Check node is a tip node.
    DEBUG_NOMSG(not start_node);
    DEBUG_NOMSG(start_node->links().size() > 1);
}

PathGenerator::PathGenerator(LinkCPtr const arrow) :
    curr_node_(arrow->src().node),
    curr_link_(arrow),
    next_node_(arrow->dst().node) {}

/* modifiers */
NodeKey PathGenerator::next()
{
    NodeKey prev_node = curr_node_;
    curr_node_ = next_node_;
    next_node_ = nullptr;
    curr_link_ = nullptr;

    // Look for next node
    if (curr_node_) {
        size_t const num_neighbors = curr_node_->links().size();
        DEBUG_NOMSG(num_neighbors > 2);

        for (auto& link : curr_node_->links()) {
            NodeKey sp = link.dst().node;
            if (sp != prev_node) {
                // curr_link links curr_node to next_node
                curr_link_ = &link;
                next_node_ = sp;
                break;
            }
        }
    }

    return curr_node_;
}
Crc32 PathGenerator::checksum() {
    DEBUG_NOMSG(curr_node_);  // At a proper start, curr_node_ is nullptr.
    Crc32 res = 0xffff;
    while (not is_done()) {
        auto nk = next();
        ProtoModule const* prot = nk->prototype_;

        // Calculate checksum from the pointer, not the value!
        checksum_cascade(&res, &prot, sizeof(prot));
    }
    return res;
}

std::vector<LinkCPtr> PathGenerator::collect_arrows()
{
    auto res = collect(
                   CollectFunc<LinkCPtr>(
    [](NodeKey const nk, LinkCPtr link) {
        return link;
    }));

    res.pop_back();  // Last link is nullptr!

    return res;
}

V3fList PathGenerator::collect_points() {
    return collect(
               CollectFunc<Vector3f>(
    [](NodeKey const nk, LinkCPtr link) {
        return nk->tx_.collapsed();
    }));
}

std::vector<NodeKey> PathGenerator::collect_keys(size_t skip) {
    return collect(
               CollectFunc<NodeKey>(
    [](NodeKey const nk, LinkCPtr link) {
        return nk;
    }),
    skip);
}

std::vector<NodeLinkPair> PathGenerator::collect_all(size_t skip) {
    return collect(
               CollectFunc<NodeLinkPair>(
    [](NodeKey const nk, LinkCPtr link) {
        return std::make_pair(nk, link);
    }),
    skip);
}

}  /* elfin */