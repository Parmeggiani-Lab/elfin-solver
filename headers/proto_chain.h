#ifndef PROTO_CHAIN_H_
#define PROTO_CHAIN_H_

#include <vector>
#include <unordered_map>

#include "proto_link.h"
#include "proto_terminus.h"

namespace elfin {

class ProtoChain {
    friend ProtoModule;
private:
    /* data members */
    bool finalized_ = false;
    ProtoTerminus n_term_;
    ProtoTerminus c_term_;

public:
    /* data members */
    const std::string name;

    /* ctors */
    ProtoChain(const std::string & _name = "unamed") :
        name(_name) {}
    ProtoChain(const ProtoChain & other) :
        ProtoChain(other.name) {}

    /* dtors */
    virtual ~ProtoChain() {}

    /* accessors */
    const ProtoTerminus & get_term(const TerminusType term) const;
    const ProtoTerminus & n_term() const { return n_term_; }
    const ProtoTerminus & c_term() const { return c_term_; }
    const ProtoLink & pick_random_proto_link(const TerminusType term) const;

    /* modifiers */
    void finalize();
};

typedef std::vector<ProtoChain> ProtoChainList;

}  /* elfin */

#endif  /* end of include guard: PROTO_CHAIN_H_ */