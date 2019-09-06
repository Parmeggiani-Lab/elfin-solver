#ifndef PROTO_CHAIN_H_
#define PROTO_CHAIN_H_

#include <vector>
#include <unordered_map>

#include "proto_link.h"
#include "proto_term.h"

namespace elfin {

/* Fwd Decl */
class Database;

class ProtoChain {
    friend ProtoModule;
    friend Database;
private:
    /* data members */
    ProtoTerm n_term_;
    ProtoTerm c_term_;

public:
    /* data members */
    std::string const name;
    size_t const id;

    /* ctors */
    ProtoChain(std::string const& _name, size_t const _id) :
        name(_name), id(_id) {
    }
    ProtoChain(ProtoChain const& other) :
        ProtoChain(other.name, other.id) {}

    /* dtors */
    virtual ~ProtoChain() {}

    /* accessors */
    ProtoTerm const& get_term(TermType const term) const;
    ProtoTerm const& n_term() const { return n_term_; }
    ProtoTerm const& c_term() const { return c_term_; }

    /* modifiers */
    void configure(std::string const& mod_name);
};

}  /* elfin */

#endif  /* end of include guard: PROTO_CHAIN_H_ */