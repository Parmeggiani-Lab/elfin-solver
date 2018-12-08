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
        if (next_node_ != nullptr) {
            UNIMPLEMENTED();
            // if (next_node_->neighbors().size() == 1) {
            //     NodeType * tmp = curr_node_;
            //     curr_node_ = next_node_;

            //     const FreeChain & free_chain =
            //         next_node_->neighbors().at(0);
            //     next_node_ = tmp == nullptr ? // was curr_node_ the beginning?
            //                  seeker.node :
            //                  nullptr;

            //     if (tmp == nullptr)
            //         wrn("Walked first node %p, next %p\n", curr_node_, next_node_);
            //     else
            //         wrn("Walked last node %p\n", tmp);
            // }
            // else {
            //     bool found_seeker = false;
            //     auto find_seeker = [&](const ChainSeekerList & csl) {
            //         if (not found_seeker) {
            //             for (const ChainSeeker & seeker : csl) {
            //                 if (seeker.node != curr_node_) {
            //                     curr_node_ = next_node_;
            //                     next_node_ = seeker.node;
            //                     found_seeker = true;
            //                     break;
            //                 }
            //             }
            //         }
            //     };
            //     find_seeker(next_node_->n_neighbors());
            //     find_seeker(next_node_->c_neighbors());

            //     DEBUG(not found_seeker,
            //           string_format(("Next seeker not found!\n"
            //                          "next_node_: %p\n%s\n"),
            //                         next_node_, next_node_->to_string().c_str()));

            // }
        }

        return curr_node_;
    }

    /* printers */

};  /* class BasicNodeGenerator*/

}  /* elfin */

#endif  /* end of include guard: BASIC_NODE_GENERATOR_H_ */