#ifndef NODES_H_
#define NODES_H_

#include <deque>

#include "node.h"

namespace elfin {

class Nodes : public std::deque<Node *> {
private:
    void destroy_nodes();

public:
    /* ctors & dtors */
    Nodes() {}
    Nodes(const Nodes & other);
    Nodes & operator=(const Nodes & other);
    virtual ~Nodes();
};

}  /* elfin */

#endif  /* end of include guard: NODES_H_ */