#ifndef MODULE_H_
#define MODULE_H_

#include <vector>
#include <unordered_map>
#include <string>
#include <tuple>

#include "jutil.h"
#include "json.h"
#include "geometry.h"
#include "string_types.h"

namespace elfin {

#define FOREACH_MODULETYPE(MACRO) \
    MACRO(SINGLE) \
    MACRO(HUB) \

GEN_ENUM_AND_STRING(ModuleType, ModuleTypeNames, FOREACH_MODULETYPE);

class Module
{
public:
    /* types */
    struct Link
    {
        Transform tx;
        Module * mod;
        Link() {}
        Link(const Transform & _tx, Module * _mod) :
            tx(_tx), mod(_mod) {}
    };
    struct Chain {
        std::string name;
        std::vector<Link> n_links;
        std::vector<Link> c_links;
        Chain(const std::string & _name) : name(_name) {}
        Chain() : name("unnamed") {}
    };
    typedef std::unordered_map<std::string, Chain> ChainMap;

protected:
    /* data members */
    ChainMap chains_;
    size_t n_link_count_ = 0;
    size_t c_link_count_ = 0;

public:
    /* data members */
    const std::string name_;
    const ModuleType type_;
    const float radius_;

    /* ctors & dtors */
    Module(const std::string & name,
           const ModuleType type,
           const float radius,
           const StrList & chain_ids) :
        name_(name), type_(type), radius_(radius) {
        for (size_t i = 0; i < chain_ids.size(); ++i) {
            const std::string & cn = chain_ids[i];
            chains_[cn] = Chain(cn);
        }
    }
    virtual ~Module() {}
    static void link_chains(
        const JSON & tx_json,
        Module * mod_a,
        const std::string & a_chain_id,
        Module * mod_b,
        const std::string & b_chain_id);

    /* getters & setters */
    const ChainMap & chains() const { return chains_; }
    const size_t n_link_count() const { return n_link_count_; }
    const size_t c_link_count() const { return c_link_count_; }
    const size_t all_link_count() const { return n_link_count_ + c_link_count_; }

    /* other methods */
    std::string to_string() const;
};

}  /* elfin */

#endif  /* end of include guard: MODULE_H_ */