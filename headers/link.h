#ifndef LINK_H_
#define LINK_H_

#include <vector>

#include "geometry.h"

namespace elfin {

struct Module;

struct Link
{
    Transform tx;
    Module * mod;
    size_t target_chain_id;

    /* ctors & dtors */
    Link(const Transform & _tx, Module * _mod, size_t chain_id) :
        tx(_tx), mod(_mod), target_chain_id(chain_id) {}

    /* other methods */
    static bool InterfaceComparator(const Link & lhs, const Link & rhs);
};

typedef std::vector<Link> LinkList;

}  /* elfin */

#endif  /* end of include guard: LINK_H_ */