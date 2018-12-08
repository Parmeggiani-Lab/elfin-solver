#ifndef BASIC_NODE_GENERATOR_H_
#define BASIC_NODE_GENERATOR_H_

#include "vector_map.h"
#include "free_chain.h"

namespace elfin {

template <class NodeType>
class BasicNodeGenerator {
private:
    /* data */
    NodeType * curr_node_ = nullptr;
    NodeType * next_node_ = nullptr;
public:
    /* ctors */
    BasicNodeGenerator(
        NodeType * start_node) :
        next_node_(start_node) {
        DEBUG(next_node_ == nullptr);
        DEBUG(next_node_->neighbors().size() != 1,
              string_format("Starting node neighbors size = %lu\n",
                            next_node_->neighbors().size()));
    }

    /* dtors */
    virtual ~BasicNodeGenerator() {}

    /* accessors */
    bool is_done() const {
        return next_node_ == nullptr;
    }

    /* modifiers */
    NodeType * next() {
        NodeType * prev_node = curr_node_;
        curr_node_ = next_node_;

        // Look for next node
        if (not is_done()) {
            DEBUG(curr_node_ == nullptr);

            const size_t neighbor_size = curr_node_->neighbors().size();
            NICE_PANIC(neighbor_size > 2);

            next_node_ = nullptr;
            for (auto & link : curr_node_->neighbors()) {
                if (link.dst().node != prev_node) {
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