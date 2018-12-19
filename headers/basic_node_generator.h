#ifndef BASIC_NODE_GENERATOR_H_
#define BASIC_NODE_GENERATOR_H_

#include "node.h"
#include "vector_map.h"

namespace elfin {

class BasicNodeGenerator {
private:
    /* data */
    NodeSP curr_node_ = nullptr;
    Link const* curr_link_ = nullptr;
    NodeSP next_node_ = nullptr;
public:
    /* ctors */
    BasicNodeGenerator(
        NodeSP const& start_node) :
        next_node_(start_node) {}

    /*
     * Constructor for starting mid-way in a basic node team.
     *
     * Note:
     *  - curr_node_ i.e. arrow source node is not included.
     *  - arrow must be a valid pointer that lives longer than the genereator
     *    itself.
     *
     */
    BasicNodeGenerator(Link const* arrow) :
        curr_node_(arrow->src().node),
        curr_link_(arrow),
        next_node_(arrow->dst().node) {}

    /* dtors */
    virtual ~BasicNodeGenerator() {}

    /* accessors */
    bool is_done() const { return next_node_ == nullptr; }

    static std::vector<Link const*> collect_arrows(
        NodeSP const& start_node);

    NodeSP curr_node() const { return curr_node_; }
    Link const* curr_link() const { return curr_link_; }

    /* modifiers */
    NodeSP next();

    /* printers */

};  /* class BasicNodeGenerator*/

}  /* elfin */

#endif  /* end of include guard: BASIC_NODE_GENERATOR_H_ */