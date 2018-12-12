#ifndef BASIC_NODE_GENERATOR_H_
#define BASIC_NODE_GENERATOR_H_

#include "node.h"
#include "vector_map.h"

namespace elfin {

class BasicNodeGenerator {
private:
    /* data */
    Node * curr_node_ = nullptr;
    const Link * curr_link_ = nullptr;
    Node * next_node_ = nullptr;
public:
    /* ctors */
    BasicNodeGenerator(
        Node * start_node) :
        next_node_(start_node) { }

    /*
     * Constructor for starting mid-way in a basic node team.
     *
     * Note:
     *  - curr_node_ i.e. arrow source node is not included.
     *  - arrow must be a valid pointer that lives longer than the genereator
     *    itself.
     *
     */
    BasicNodeGenerator(const Link * arrow) :
        curr_node_(arrow->src().node),
        curr_link_(arrow),
        next_node_(arrow->dst().node) {}

    /* dtors */
    virtual ~BasicNodeGenerator() {}

    /* accessors */
    bool is_done() const {
        return next_node_ == nullptr;
    }

    Node * curr_node() const { return curr_node_; }
    const Link * curr_link() const { return curr_link_; }

    /* modifiers */
    Node * next() {
        Node * prev_node = curr_node_;
        curr_node_ = next_node_;

        next_node_ = nullptr;
        curr_link_ = nullptr;

        // Look for next node
        if (curr_node_) {
            const size_t neighbor_size = curr_node_->links().size();
            NICE_PANIC(neighbor_size > 2);
            for (auto & link : curr_node_->links()) {
                if (link.dst().node != prev_node) {
                    // curr_link links curr_node to next_node
                    curr_link_ = &link;
                    next_node_ = link.dst().node;
                    break;
                }
            }
        }

        return curr_node_;
    }

    /* printers */

};  /* class BasicNodeGenerator*/

}  /* elfin */

#endif  /* end of include guard: BASIC_NODE_GENERATOR_H_ */