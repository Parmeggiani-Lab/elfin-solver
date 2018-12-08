#ifndef NODE_H_
#define NODE_H_

#include <unordered_set>

#include "proto_module.h"
#include "geometry.h"
#include "terminus_type.h"
#include "link.h"

namespace elfin {

class Node
{
protected:
    /*
     * Note: Use of vector as container for neighbors (LinkList)
     *
     * I chose to use vector because as far as we are concerned now, modules
     * have very small number of termini e.g. max 4 at the time of writing.
     * Although it's O(n) to access the wanted FreeChain, using more complex
     * structure to support O(1) operations is probably not worth the
     * memory/dev time due to how tiny the vectors are.
     *
     * If in the future the number of termini per module does increase, it
     * might be advisable to switch to VectorMap or something better.
     */

    /* data */
    const ProtoModule * prototype_ = nullptr;
    Transform tx_;
    LinkList neighbors_;
public:
    /* ctors */
    Node(const ProtoModule * prototype, const Transform & tx);
    Node(const ProtoModule * prototype) : Node(prototype, Transform()) {}
    virtual Node * clone() const { return new Node(*this); }

    /* dtors */
    virtual ~Node() {}

    /* accessors */
    const ProtoModule * prototype() const { return prototype_; }
    const LinkList & neighbors() const { return neighbors_; }
    const Transform & tx() const { return tx_; }

    /* modifiers */
    Transform & tx() { return tx_; }
    void add_link(
        const FreeChain & src,
        const FreeChain & dst) {
        neighbors_.emplace_back(src, dst);
        const Link & l = neighbors_.back();
    }
    void update_neighbor_ptrs(const NodeAddrMap & nam);
    void remove_link(const Link link);

    /* printers */
    virtual std::string to_string() const;
    virtual std::string to_csv_string() const;
};

}  /* elfin */

#endif  /* end of include guard: NODE_H_ */