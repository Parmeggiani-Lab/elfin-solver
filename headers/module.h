#ifndef MODULE_H_
#define MODULE_H_

#include <vector>
#include <unordered_map>
#include <string>
#include <tuple>

#include "json.h"
#include "geometry.h"
#include "string_types.h"

namespace elfin {

#define FOREACH_MODULETYPE(MACRO) \
    MACRO(SINGLE) \
    MACRO(HUB) \

GEN_ENUM_AND_STRING(ModuleType, ModuleTypeNames, FOREACH_MODULETYPE);

#define FOREACH_TERMINUSTYPE(MACRO) \
    MACRO(N) \
    MACRO(C) \
    MACRO(ENUM_COUNT) \

GEN_ENUM_AND_STRING(TerminusType, TerminusTypeNames, FOREACH_TERMINUSTYPE);

static_assert(TerminusType::ENUM_COUNT == 2);

class Module
{
public:
    /* types */
    typedef std::tuple<Transform, Module *> TxMod;
    struct Chain {
        std::string name;
        std::vector<TxMod> n_links;
        std::vector<TxMod> c_links;
        Chain(const std::string & _name) : name(_name) {}
    };
    typedef std::unordered_map<std::string, Chain> ChainMap;
    struct Radii {
        float avg_all;
        float max_ca;
        float max_heavy;
    } radii;

protected:
    /* data members */
    ChainMap chains_;
    size_t n_link_count_, c_link_count_;
    Radii radii_;

    /* ctors & dtors */
    Module() {}

public:
    /* data members */
    const std::string name_;
    const ModuleType type_;

    /* ctors & dtors */
    Module(const std::string & name,
           const ModuleType type,
           const StrList & chain_names) :
        name_(name), type_(type) {
        for (size_t i = 0; i < chain_names.size(); ++i) {
            const std::string & cn = chain_names[i];
            chains_[cn] = Chain(cn);
        }
    }
    virtual ~Module() {}
    static void link_chains(
        const JSON & tx_json,
        Chain & a_chain,
        Chain & b_chain);

    /* getters & setters */
    const ChainMap & chains() const { return chains_; }
    const size_t n_link_count() const { return n_link_count_; }
    const size_t c_link_count() const { return c_link_count_; }
    const size_t all_link_count() const { return n_link_count_ + c_link_count; }
    const Radii & radii() const { return radii_; }
    const void set_radii(const Radii & radii) { radii_ = radii; }

    /* other methods */
    std::string to_string() const;
};

}  /* elfin */

#endif  /* end of include guard: MODULE_H_ */