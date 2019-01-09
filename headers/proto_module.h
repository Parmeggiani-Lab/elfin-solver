#ifndef PROTO_MODULE_H_
#define PROTO_MODULE_H_

#include <memory>
#include <string>

#include "json.h"
#include "proto_chain.h"
#include "free_term.h"

namespace elfin {

/* Fwd Decl */
struct FreeTerm;
class ProtoModule;
typedef ProtoModule const* PtModKey;
class Database;

/* types */
#define FOREACH_MODULETYPE(MACRO) \
    MACRO(SINGLE) \
    MACRO(ASYM_HUB) \
    MACRO(SYM_HUB) \
    MACRO(UNKNOWN)
GEN_ENUM_AND_STRING(ModuleType, ModuleTypeNames, FOREACH_MODULETYPE);

/* free function */
bool is_hub(ModuleType const type);

class ProtoModule : public Printable {
    friend Database;
public:
    /* types */
    struct Counts {
        size_t n_links = 0, c_links = 0;
        size_t n_interfaces = 0, c_interfaces = 0;
        size_t all_links() const { return n_links + c_links; }
        size_t all_interfaces() const { return n_interfaces + c_interfaces; }
    };

private:
    /* types */
    typedef std::vector<ProtoChain> PtChains;

    /* data */
    bool already_finalized_ = false;
    PtChains chains_;
    FreeTerms free_terms_;
    Counts counts_ = {};

public:
    /* data */
    std::string const name;
    ModuleType const type;
    float const radius;

    /* ctors */
    ProtoModule(std::string const& name,
                ModuleType const type,
                float const radius,
                StrList const& chain_names);

    /* accessors */
    PtChains const& chains() const { return chains_; }
    ProtoChain const& get_chain(size_t const id) const;
    ProtoChain const& get_chain(std::string const name) const {
        return chains_.at(get_chain_id(name));
    }
    FreeTerms const& free_terms() const { return free_terms_; }
    Counts const& counts() const { return counts_; }
    size_t get_chain_id(std::string const& chain_name) const;
    ProtoLink const* find_link_to(size_t const src_chain_id,
                                  TermType const src_term,
                                  PtModKey const dst_mod,
                                  size_t const dst_chain_id) const;
    bool is_hub() const { return elfin::is_hub(type); }
    ProtoTerm const& get_term(FreeTerm const& ft) const;
    PtTermKeySet get_reachable_ptterms(FreeTerms const& src_terms) const;

    /* modifiers */
    void finalize();
    static void create_proto_link_pair(JSON const& xdb_json,
                                       size_t const tx_id,
                                       ProtoModule& mod_a,
                                       std::string const& a_chain_name,
                                       ProtoModule& mod_b,
                                       std::string const& b_chain_name);

    /* printers */
    virtual void print_to(std::ostream& os) const;
};

typedef std::unique_ptr<ProtoModule> ProtoModuleSP;

}  /* elfin */

namespace std {
template <>
struct hash<elfin::PtModKey>
{
    size_t operator()(elfin::PtModKey const& key) const {
        return hash<void*>()((void*) key);
    }
};
}

#endif  /* end of include guard: PROTO_MODULE_H_ */