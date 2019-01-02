#ifndef PATH_GENERATOR_H_
#define PATH_GENERATOR_H_

#include <vector>
#include <functional>

#include "checksum.h"
#include "debug_utils.h"

namespace elfin {

/* Fwd Decl */
class Link;
class Node;
typedef Node const* NodeKey;
struct TestStat;
class Vector3f;
typedef std::vector<Vector3f> V3fList;
typedef Link const* LinkCPtr;

class PathGenerator {
private:
    /* data */
    NodeKey curr_node_ = nullptr;
    LinkCPtr curr_link_ = nullptr;
    NodeKey next_node_ = nullptr;

public:
    /* types */
    template<typename T>
    using CollectFunc = std::function<T(NodeKey, LinkCPtr)>;

    /* ctors */
    // Ctor for starting at a tip node - assertion is enforced.
    PathGenerator(NodeKey start_node);


    // Ctor for starting mid-way in a basic node team.
    //  - curr_node_ i.e. arrow source node is not included.
    //  - arrow must be a valid pointer that lives longer than the genereator
    //    itself.
    PathGenerator(LinkCPtr const arrow);

    /* dtors */
    virtual ~PathGenerator() {}

    /* accessors */
    bool is_done() const { return not next_node_; }
    NodeKey curr_node() const { return curr_node_; }
    NodeKey peek() const { return next_node_; }
    LinkCPtr curr_link() const { return curr_link_; }

    /* modifiers */
    NodeKey next();
    Crc32 checksum();
    std::vector<LinkCPtr> collect_arrows();
    V3fList collect_points();
    std::vector<NodeKey> collect_keys();

    template<typename T, template <typename... Args> class C = std::vector>
    C<T> collect(CollectFunc<T> const& func) {
        DEBUG_NOMSG(curr_node_);  // At a proper start, curr_node_ is nullptr.
        C<T> res;
        while (not is_done()) {
            NodeKey nk = next();
            res.push_back(func(nk, curr_link_));
        }
        return res;
    }

    /* printers */

    /* tests */
    static TestStat test();
};  /* class PathGenerator */

}  /* elfin */

#endif  /* end of include guard: PATH_GENERATOR_H_ */