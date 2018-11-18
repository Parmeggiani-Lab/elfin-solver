#ifndef MODULE_H_
#define MODULE_H_

#include <vector>
#include <string>
#include <tuple>

#include "json.h"
#include "geometry.h"
#include "map_types.h"

namespace elfin {

#define FOREACH_MODULETYPE(MACRO) \
    MACRO(SINGLE) \
    MACRO(HUB) \

GEN_ENUM_AND_STRING(ModuleType, ModuleTypeNames, FOREACH_MODULETYPE);

#define FOREACH_TERMINUSTYPE(MACRO) \
    MACRO(N) \
    MACRO(C) \

GEN_ENUM_AND_STRING(TerminusType, TerminusTypeNames, FOREACH_TERMINUSTYPE);

class Module
{
public:
    /* types */
    typedef std::tuple<Transform, Module *> TxMod;
    struct Chain {
        std::vector<TxMod> n_links;
        std::vector<TxMod> c_links;
    };

protected:

    /* data members */
    std::vector<Chain> chains_;
    size_t n_link_count_, c_link_count_;

    /* ctors & dtors */
    Module() {}

public:
    /* data members */
    const std::string name_;
    const ModuleType type_;
    const IdStrMap chain_itn_;

    /* ctors & dtors */
    Module(const str::string & name,
           const ModuleType type,
           const IdStrMap & chain_itn) :
        name_(name),
        type_(type),
        chain_itn_(chain_itn)
    {
        const Chain def_chain;
        chains_.resize(chain_itn_.size(), def_chain);
    }
    virtual ~Module() {}
    static void link_modules(
        const JSON & tx_json,
        Module * amod,
        const size_t amod_chain_id
        Module * bmod,
        const size_t bmod_chain_id);

    /* getters & setters */
    const std::vector<Chain> & chains() const { return chains_; }
    const size_t n_link_count() const { return n_link_count_; }
    const size_t c_link_count() const { return c_link_count_; }
    const size_t all_link_count() const { return n_link_count_ + c_link_count; }

    /* other methods */
    std::string to_string() const;
};

}  /* elfin */

#endif  /* end of include guard: MODULE_H_ */