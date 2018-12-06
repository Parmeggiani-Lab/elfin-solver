#ifndef CHAIN_H_
#define CHAIN_H_

#include <vector>
#include <unordered_map>

#include "link.h"
#include "terminus_type.h"
#include "terminus.h"

namespace elfin {

class Chain {
    friend Module;
private:
    /* data members */
    bool finalized_ = false;
    Terminus n_term_;
    Terminus c_term_;

public:
    /* data members */
    const std::string name;

    /* ctors & dtors */
    Chain(const std::string & _name = "unamed") :
        name(_name) {}
    Chain(const Chain & other) :
        Chain(other.name) {}

    /* getters & setters */
    const Terminus & get_term(const TerminusType term) const;
    const Terminus & n_term() const { return n_term_; }
    const Terminus & c_term() const { return c_term_; }

    /* other methods */
    void finalize();
    const Link & pick_random_link(const TerminusType term) const;
};

typedef std::vector<Chain> ChainList;

}  /* elfin */

#endif  /* end of include guard: CHAIN_H_ */