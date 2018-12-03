#ifndef NODES_H_
#define NODES_H_

#include <deque>

#include "node.h"

namespace elfin {

typedef std::deque<Node *> NodesContainer;

class Nodes : public NodesContainer {
public:
    /* ctors & dtors */
    Nodes() {}
    Nodes(const Nodes & other);
    Nodes & operator=(const Nodes & other);
    virtual ~Nodes();

    /* other methods */
    void clear();
};

}  /* elfin */

#endif  /* end of include guard: NODES_H_ */