#ifndef NODE_H_
#define NODE_H_

#include <unordered_set>

#include "proto_module.h"
#include "geometry.h"
#include "terminus_type.h"
#include "chain_seeker.h"

namespace elfin {

class Node
{
protected:
    /*
     * Note: Use of vector as container for neighbors (ChainSeekerList)
     *
     * I chose to use vector because as far as we are concerned now, modules
     * have very small number of termini e.g. max 4 at the time of writing.
     * Although it's O(n) to access the wanted ChainSeeker, using more complex
     * structure to support O(1) operations is probably not worth the
     * memory/dev time due to how tiny the vectors are.
     *
     * If in the future the number of termini per module does increase, it
     * might be advisable to switch to VectorMap or something better.
     */

    /* data */
    const ProtoModule * prototype_ = nullptr;
    Transform tx_;
    ChainSeekerList n_neighbors_, c_neighbors_;
public:
    /* ctors */
    Node(const ProtoModule * prototype, const Transform & tx);
    Node(const ProtoModule * prototype) : Node(prototype, Transform()) {}
    virtual Node * clone() const { return new Node(*this); }

    /* dtors */
    virtual ~Node() {}

    /* accessors */
    const ProtoModule * prototype() const { return prototype_; }
    const ChainSeekerList & n_neighbors() const { return n_neighbors_; }
    const ChainSeekerList & c_neighbors() const { return c_neighbors_; }
    const Transform & tx() const { return const_cast<Node *>(this)->tx(); }

    /* modifiers */
    Transform & tx() { return tx_; }
    void store_seeker(
        const ChainSeeker & seeker,
        const TerminusType term);

    /* printers */
    virtual std::string to_string() const;
    virtual std::string to_csv_string() const;
};

}  /* elfin */

#endif  /* end of include guard: NODE_H_ */