#ifndef PROTO_MODULE_H_
#define PROTO_MODULE_H_

#include <vector>
#include <unordered_map>
#include <string>
#include <tuple>

#include "json.h"
#include "proto_chain.h"
#include "debug_utils.h"
#include "vector_utils.h"
#include "link.h"

namespace elfin {

#define FOREACH_MODULETYPE(MACRO) \
    MACRO(SINGLE) \
    MACRO(HUB) \
    MACRO(NOT_MODULE) \

GEN_ENUM_AND_STRING(ModuleType, ModuleTypeNames, FOREACH_MODULETYPE);

class ProtoModule {
public:
    /* types */
    struct Counts {
        size_t n_links = 0, c_links = 0;
        size_t n_interfaces = 0, c_interfaces = 0;
        size_t all_links() const { return n_links + c_links; }
        size_t all_interfaces() const { return n_interfaces + c_interfaces; }
    };

private:
    /* data */
    bool finalized_ = false;
    ProtoChainList chains_;
    Counts counts_ = {};

public:
    /* types */
    struct Bridge {
        ProtoLink const* const ptlink1, * const ptlink2;
        Bridge(ProtoLink const* _ptlink1, ProtoLink const*_ptlink2) :
            ptlink1(_ptlink1), ptlink2(_ptlink2) {}
    };

    typedef Vector<Bridge> BridgeList;

    /* data */
    std::string const name;
    ModuleType const type;
    float const radius;

    /* ctors */
    ProtoModule(
        std::string const& name,
        ModuleType const type,
        float const radius,
        StrList const& chain_names);

    /* dtors */
    virtual ~ProtoModule() {}

    /* accessors */
    ProtoChainList const& proto_chains() const { return chains_; }
    Counts const& counts() const { return counts_; }
    size_t find_chain_id(std::string const& chain_name) const;
    ProtoLink const* find_link_to(
        size_t const src_chain_id,
        TerminusType const src_term,
        ProtoModule const* dst_module,
        size_t const dst_chain_id) const;
    BridgeList find_bridges(Link const* arrow) const;

    /* modifiers */
    void finalize();
    static void create_proto_link_pair(
        JSON const& tx_json,
        ProtoModule* mod_a,
        std::string const& a_chain_name,
        ProtoModule* mod_b,
        std::string const& b_chain_name);

    /* printers */
    std::string to_string() const;
};

}  /* elfin */

#endif  /* end of include guard: PROTO_MODULE_H_ */