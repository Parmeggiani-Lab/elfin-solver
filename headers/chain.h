#ifndef CHAIN_H_
#define CHAIN_H_

#include <vector>

#include "link.h"
#include "terminus_type.h"

namespace elfin {

class Chain {
    friend Module;
private:
    /* data members */
    LinkList n_links_;
    LinkList c_links_;

public:
    /* data members */
    const std::string name;
    const LinkList & n_links = n_links_;
    const LinkList & c_links = c_links_;

    /* ctors & dtors */
    Chain(const std::string & _name="unamed") : name(_name) {}

    /* other methods */
    const LinkList & get_links(const TerminusType term) const;
    void finalize();
    const Link & pick_random_link() const;
};

typedef std::vector<Chain> ChainList;

}  /* elfin */

#endif  /* end of include guard: CHAIN_H_ */