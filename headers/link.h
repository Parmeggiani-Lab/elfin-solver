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

    /* ctors */
    Link(const Transform & _tx, Module * _mod, const size_t chain_id) :
        tx(_tx), mod(_mod), target_chain_id(chain_id) {}

    /* dtors */
    virtual ~Link() {}

    /* operators  */
    bool operator<(const Link & rhs) const;
};

typedef std::vector<Link> LinkList;

}  /* elfin */

#endif  /* end of include guard: LINK_H_ */