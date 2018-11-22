#ifndef NODE_LIST_H_
#define NODE_LIST_H_

#include <deque>

#include "node.h"

namespace elfin {

class NodeList : public std::deque<Node> {
private:
    static size_t MAX_LEN_;


    bool collides(
        const Vector3f & new_com,
        const float mod_radius) const;
    bool pick_sever_point(
        size_t & id_out,
        const TerminusType term);
    void randomize(
        NodeList::const_iterator head,
        const TerminusType term,
        const size_t n_nodes);

public:
    /* data members */
    static const size_t & MAX_LEN;
    static void set_max_len(const size_t l) { MAX_LEN_ = l; }

    /* other methods */
    void destructive_randomize();
    void auto_mutate();
    bool cross_mutate(const NodeList & father);
    bool point_mutate();
    bool limb_mutate();
};

}  /* elfin */

#endif  /* end of include guard: NODE_LIST_H_ */