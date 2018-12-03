#ifndef MODULE_H_
#define MODULE_H_

#include <vector>
#include <unordered_map>
#include <string>
#include <tuple>

#include "jutil.h"
#include "json.h"
#include "string_utils.h"
#include "chain.h"
#include "debug_utils.h"

namespace elfin {

#define FOREACH_MODULETYPE(MACRO) \
    MACRO(SINGLE) \
    MACRO(HUB) \
    MACRO(NOT_MODULE) \

GEN_ENUM_AND_STRING(ModuleType, ModuleTypeNames, FOREACH_MODULETYPE);

class Module
{
public:
    /* types */
    struct Counts {
        size_t n_link = 0, c_link = 0, interface = 0;
        size_t all_link() const { return n_link + c_link; }
    };

private:
    ChainList chains_;
    StrIndexMap chain_id_map_;

    Counts counts_ = {};

public:
    /* data members */
    const std::string name;
    const ModuleType type;
    const float radius;
    const StrList chain_names;
    const ChainList & chains = chains_;
    const StrIndexMap & chain_id_map = chain_id_map_;

    // finalizer fields
    const Counts & counts = counts_;

    /* ctors & dtors */
    Module(const std::string & name,
           const ModuleType type,
           const float radius,
           const StrList & chain_names);
    ~Module() {
        err("?\n");
        DEBUG("Dtor\b");
    }

    /* other methods */
    void finalize();
    std::string to_string() const;
    static void create_link(
        const JSON & tx_json,
        Module * mod_a,
        const std::string & a_chain_name,
        Module * mod_b,
        const std::string & b_chain_name);
};

}  /* elfin */

#endif  /* end of include guard: MODULE_H_ */