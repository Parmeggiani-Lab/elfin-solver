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
    TerminusTracker free_term_;

public:
    /* ctors & dtors */
    Node(const Module * prototype, const Transform & tx);
    Node(const Module * prototype) : Node(prototype, Transform()) {}
    virtual Node * clone() const {
        return new Node(*this);
    }

    /* getters & setters */
    const Module * prototype() const { return prototype_; }
    Transform & tx() { return tx_; }
    const TerminusTracker & free_term() const { return free_term_; }
    void occupy_terminus(TerminusType term, size_t chain_id);
    void free_terminus(TerminusType term, size_t chain_id);

    /* other methods */
    virtual std::string to_string() const;
    virtual std::string to_csv_string() const;
};

typedef std::deque<Node *> NodeDeque;

}  /* elfin */

#endif  /* end of include guard: NODE_H_ */