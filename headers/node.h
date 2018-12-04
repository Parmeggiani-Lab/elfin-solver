#ifndef NODE_H_
#define NODE_H_

#include <unordered_set>

#include "module.h"
#include "geometry.h"
#include "terminus_type.h"
#include "terminus_tracker.h"

namespace elfin {

class Node
{
protected:
    /* data members */
    const Module * prototype_ = nullptr;
    Transform tx_ = {};
    TerminusTracker term_tracker_;
public:
    /* ctors & dtors */
    Node(const Module * prototype, const Transform & tx);
    Node(const Module * prototype) : Node(prototype, Transform()) {}
    virtual Node * clone() const { return new Node(*this); }

    /* getters & setters */
    const Module * prototype() const { return prototype_; }
    Transform & tx() { return tx_; }
    const TerminusTracker & term_tracker() const {
        return term_tracker_;
    }
    static void connect(
        Node * node_a,
        const size_t a_chain_id,
        const TerminusType a_term,
        Node * node_b,
        const size_t b_chain_id);
    // static void disconnect(
    //     Node * node_a,
    //     ?,
    //     Node * node_b);

    /* other methods */
    virtual std::string to_string() const;
    virtual std::string to_csv_string() const;
};

}  /* elfin */

#endif  /* end of include guard: NODE_H_ */