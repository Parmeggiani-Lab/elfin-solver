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

PathGenerator::PathGenerator(Link const* const arrow) :
    curr_node_(arrow->src().node),
    curr_link_(arrow),
    next_node_(arrow->dst().node) {}

/* accessors */
std::vector<Link const*> PathGenerator::collect_arrows() const
{
    DEBUG_NOMSG(curr_node_);  // At a proper start, curr_node_ is nullptr.
    auto pg = PathGenerator(next_node_);
    std::vector<Link const*> arrows;
    while (not pg.is_done()) {
        pg.next();
        if (pg.curr_link()) {  // The last call to .next() has nullptr for curr_link.
            arrows.push_back(pg.curr_link());
        }
    }
    return arrows;
}

Crc32 PathGenerator::path_checksum() const {
    DEBUG_NOMSG(curr_node_);  // At a proper start, curr_node_ is nullptr.
    auto pg = PathGenerator(next_node_);
    Crc32 res = 0xffff;
    while (not pg.is_done()) {
        auto nk = pg.next();
        ProtoModule const* prot = nk->prototype_;

        // Calculate checksum from the pointer, not the value!
        checksum_cascade(&res, &prot, sizeof(prot));
    }
    return res;
}

V3fList PathGenerator::collect_points() const {
    DEBUG_NOMSG(curr_node_);  // At a proper start, curr_node_ is nullptr.
    auto pg = PathGenerator(next_node_);
    V3fList points;
    while (not pg.is_done()) {
        points.push_back(pg.next()->tx_.collapsed());
    }
    return points;
}

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
        DEBUG_NOMSG(num_neighbors == 0);

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

}  /* elfin */