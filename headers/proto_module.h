#ifndef PROTO_MODULE_H_
#define PROTO_MODULE_H_

#include <vector>
#include <unordered_map>
#include <string>
#include <tuple>

#include "jutil.h"
#include "json.h"
#include "string_utils.h"
#include "proto_chain.h"
#include "debug_utils.h"

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
        size_t n_link = 0, c_link = 0;
        size_t n_interface = 0, c_interface = 0;
        size_t all_links() const { return n_link + c_link; }
        size_t all_interfaces() const { return n_interface + c_interface; }
    };

private:
    /* data */
    bool finalized_ = false;
    ProtoChainList chains_;
    StrIndexMap chain_id_map_;
    Counts counts_ = {};

public:
    /* data */
    const std::string name;
    const ModuleType type;
    const float radius;
    const StrList chain_names;
    const ProtoChainList & proto_chains() const { return chains_; }
    const StrIndexMap & chain_id_map() const { return chain_id_map_; }
    const Counts & counts() const { return counts_; }

    /* ctors */
    ProtoModule(const std::string & name,
           const ModuleType type,
           const float radius,
           const StrList & chain_names);

    /* dtors */
    virtual ~ProtoModule() {}

    /* modifiers */
    void finalize();
    static void create_proto_link(
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