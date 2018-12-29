#ifndef PATH_GENERATOR_H_
#define PATH_GENERATOR_H_

#include <vector>

namespace elfin {

/* Fwd Decl */
class Link;
class Node;
typedef Node const* NodeKey;

class PathGenerator {
private:
    /* data */
    NodeKey curr_node_ = nullptr;
    Link const* curr_link_ = nullptr;
    NodeKey next_node_ = nullptr;
public:
    /* ctors */
    PathGenerator(NodeKey start_node) :
        next_node_(start_node) {}

    //
    // Constructor for starting mid-way in a basic node team.
    //  - curr_node_ i.e. arrow source node is not included.
    //  - arrow must be a valid pointer that lives longer than the genereator
    //    itself.
    //
    //
    PathGenerator(Link const* const arrow);

    /* dtors */
    virtual ~PathGenerator() {}

    /* accessors */
    bool is_done() const { return not next_node_; }
    std::vector<Link const*> collect_arrows();
    NodeKey curr_node() const { return curr_node_; }
    NodeKey peek() const { return next_node_; }
    Link const* curr_link() const { return curr_link_; }

    /* modifiers */
    NodeKey next();

    /* printers */
};  /* class PathGenerator */

}  /* elfin */

#endif  /* end of include guard: PATH_GENERATOR_H_ */