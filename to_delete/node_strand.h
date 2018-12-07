#ifndef NODE_STRAND_H_
#define NODE_STRAND_H_

#include <deque>

#include "node.h"

namespace elfin {

typedef std::deque<Node> NodeDeque;
typedef NodeDeque::iterator NodeDequeItr;

class NodeStrand {
private:
    /* data */
    NodeDeque nodes_;
public:
    /* ctors */
    NodeStrand() {}
    NodeStrand(const ProtoModule * start_mod);

    /* dtors */
    virtual ~NodeStrand() {}

    /* accessors */
    size_t size() const { return nodes_.size(); }
    const NodeDeque & nodes() const { return nodes_; }
    V3fList to_points() const;
    const Node & random_tip() const;

    /* modifiers */

    /* printers */
    StrList get_node_names() const;

};  /* class NodeStrand*/

}  /* elfin */

#endif  /* end of include guard: NODE_STRAND_H_ */