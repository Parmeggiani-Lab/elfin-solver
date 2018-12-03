#ifndef CHAIN_H_
#define CHAIN_H_

#include <vector>
#include <unordered_map>

#include "link.h"
#include "terminus_type.h"
#include "terminus.h"\

namespace elfin {

class Chain {
    friend Module;
private:
    /* data members */
    Terminus n_term_;
    Terminus c_term_;

public:
    /* data members */
    const std::string name;
    const Terminus & n_term = n_term_;
    const Terminus & c_term = c_term_;

    /* ctors & dtors */
    Chain(const std::string & _name = "unamed") : name(_name) {}
    const Terminus & get_term(const TerminusType term) const;

    /* other methods */
    void finalize();
    const Link & pick_random_link(const TerminusType term) const;
};

typedef std::vector<Chain> ChainList;

}  /* elfin */

#endif  /* end of include guard: CHAIN_H_ */