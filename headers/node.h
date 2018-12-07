#ifndef NODE_H_
#define NODE_H_

#include <unordered_set>

#include "proto_module.h"
#include "geometry.h"
#include "terminus_type.h"

namespace elfin {

class Node
{
protected:
    /* types */
    struct Neighbor {
        size_t chain_id;
        const ProtoLink * proto_link = nullptr;
    };
    typedef std::vector<Neighbor> NeighborList;

    /* data */
    const ProtoModule * prototype_ = nullptr;
    Transform tx_ = {};
    NeighborList n_neighbors_, c_neighbors_;
public:
    /* ctors */
    Node(const ProtoModule * prototype, const Transform & tx);
    Node(const ProtoModule * prototype) : Node(prototype, Transform()) {}
    virtual Node * clone() const { return new Node(*this); }

    /* dtors */
    virtual ~Node() {}

    /* accessors */
    const ProtoModule * prototype() const { return prototype_; }
    const Transform & tx() const { return const_cast<Node *>(this)->tx(); }

    /* modifiers */
    Transform & tx() { return tx_; }

    /* printers */
    virtual std::string to_string() const;
    virtual std::string to_csv_string() const;
};

}  /* elfin */

#endif  /* end of include guard: NODE_H_ */