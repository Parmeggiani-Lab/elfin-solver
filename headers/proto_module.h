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
    /* data */
    const std::string name;
    const ModuleType type;
    const float radius;
    const ProtoChainList & proto_chains() const { return chains_; }
    const Counts & counts() const { return counts_; }

    /* ctors */
    ProtoModule(const std::string & name,
                const ModuleType type,
                const float radius,
                const StrList & chain_names);

    /* dtors */
    virtual ~ProtoModule() {}

    /* accessors */
    size_t find_chain_id(const std::string & chain_name) const;
    const ProtoLink * find_link_to(
        const size_t src_chain_id,
        const TerminusType src_term,
        const ProtoModule * dst_module,
        const size_t dst_chain_id) const;
    // bool has_link_to(
    //     const TerminusType src_term,
    //     ConstProtoModulePtr dst_module,
    //     const size_t dst_chain_id) const;
    Vector<const ProtoModule *>
    find_intermediate_proto_modules_to(
        const size_t src_chain_id,
        const TerminusType src_term,
        const ProtoModule * dst_module,
        const size_t dst_chain_id) const;

    /* modifiers */
    void finalize();
    static void create_proto_link_pair(
        const JSON & tx_json,
        ProtoModule * mod_a,
        const std::string & a_chain_name,
        ProtoModule * mod_b,
        const std::string & b_chain_name);

    /* printers */
    std::string to_string() const;
};

}  /* elfin */

#endif  /* end of include guard: PROTO_MODULE_H_ */