#ifndef NODE_H_
#define NODE_H_

#include "module.h"
#include "geometry.h"
#include "terminus_type.h"

namespace elfin {

struct Node {
    /* data members */
    const Module * prototype;
    Transform tx;

    /* ctors & dtors */
    Node(const Module * _prototype, const Transform & _tx) :
        prototype(_prototype), tx(_tx) {}
    Node() {}

    /* other methods */
    std::string to_string() const;
    std::string to_csv_string() const;
};

}  /* elfin */

#endif  /* end of include guard: NODE_H_ */