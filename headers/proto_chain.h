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
    std::string const name;
    size_t const id;

    /* ctors */
    ProtoChain(std::string const& _name, size_t const _id) :
        name(_name), id(_id) {}
    ProtoChain(ProtoChain const& other) :
        ProtoChain(other.name, other.id) {}

    /* dtors */
    virtual ~ProtoChain() {}

    /* accessors */
    ProtoTerminus const& get_term(TerminusType const term) const;
    ProtoTerminus const& n_term() const { return n_term_; }
    ProtoTerminus const& c_term() const { return c_term_; }
    ProtoLink const& pick_random_link(
        TerminusType const term) const;

    /* modifiers */
    void finalize();
};

typedef std::vector<ProtoChain> ProtoChainList;

}  /* elfin */

#endif  /* end of include guard: PROTO_CHAIN_H_ */