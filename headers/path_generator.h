#ifndef PATH_GENERATOR_H_
#define PATH_GENERATOR_H_

#include <vector>

#include "checksum.h"

namespace elfin {

/* Fwd Decl */
class Link;
class Node;
typedef Node const* NodeKey;
struct TestStat;
class Vector3f;
typedef std::vector<Vector3f> V3fList;

class PathGenerator {
private:
    /* data */
    NodeKey curr_node_ = nullptr;
    Link const* curr_link_ = nullptr;
    NodeKey next_node_ = nullptr;
public:
    /* ctors */
    PathGenerator(NodeKey start_node);

    //
    // Constructor for starting mid-way in a basic node team.
    //  - curr_node_ i.e. arrow source node is not included.
    //  - arrow must be a valid pointer that lives longer than the genereator
    //    itself.
    //
    PathGenerator(Link const* const arrow);

    /* dtors */
    virtual ~PathGenerator() {}

    /* accessors */
    bool is_done() const { return not next_node_; }
    NodeKey curr_node() const { return curr_node_; }
    NodeKey peek() const { return next_node_; }
    Link const* curr_link() const { return curr_link_; }
    std::vector<Link const*> collect_arrows() const;
    Crc32 path_checksum() const;
    V3fList collect_points(size_t skip = 0) const;

    /* modifiers */
    NodeKey next();

    /* printers */

    /* tests */
    static TestStat test();
};  /* class PathGenerator */

}  /* elfin */

#endif  /* end of include guard: PATH_GENERATOR_H_ */