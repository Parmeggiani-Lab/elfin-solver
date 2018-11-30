#ifndef MODULE_H_
#define MODULE_H_

#include <vector>
#include <unordered_map>
#include <string>
#include <tuple>

#include "jutil.h"
#include "json.h"
#include "geometry.h"
#include "string_utils.h"
#include "terminus_type.h"

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
        size_t target_chain_id;
        Link() {}
        Link(const Transform & _tx, Module * _mod, size_t chain_id) :
            tx(_tx), mod(_mod), target_chain_id(chain_id) {}
    };
    struct Chain {
        std::string name = "unamed";
        std::vector<Link> n_links;
        std::vector<Link> c_links;
        Chain(const std::string & _name) : name(_name) {}
        Chain() {}
        const std::vector<Link> & get_links(TerminusType term) const {
            if (term == TerminusType::N) {
                return n_links;
            }
            else if (term == TerminusType::C) {
                return c_links;
            }
            else {
                death_by_bad_terminus(__PRETTY_FUNCTION__, term);
            }
        }
    };
    typedef std::vector<Chain> ChainList;

protected:
    /* data members */
    ChainList chains_;
    size_t n_link_count_ = 0;
    size_t c_link_count_ = 0;
    size_t interface_count_ = 0;
    StrList chain_names_;
    StrIndexMap chain_id_map_;

public:
    /* types */
    typedef size_t (*CmlSumFunctor)(Module *&);
    
    /* data members */
    const std::string name_;
    const ModuleType type_;
    const float radius_;

    /* ctors & dtors */
    Module(const std::string & name,
           const ModuleType type,
           const float radius,
           const StrList & chain_names) :
        name_(name), type_(type), radius_(radius), chain_names_(chain_names) {
        for (size_t i = 0; i < chain_names.size(); ++i) {
            const std::string & cn = chain_names[i];

            assert(i == chains_.size());
            chain_id_map_[cn] = chains_.size();
            chains_.emplace_back(cn);
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
    const ChainList & chains() const { return chains_; }
    void finalize();
    size_t n_link_count() const { return n_link_count_; }
    size_t c_link_count() const { return c_link_count_; }
    size_t all_link_count() const { return n_link_count_ + c_link_count_; }
    size_t interface_count() const { return interface_count_; }
    const StrList & chain_names() const { return chain_names_; }
    const StrIndexMap & chain_id_map() const { return chain_id_map_; }

    static CmlSumFunctor n_link_count_functor;
    static CmlSumFunctor c_link_count_functor;
    static CmlSumFunctor all_link_count_functor;

    /* other methods */
    std::string to_string() const;
};

}  /* elfin */

#endif  /* end of include guard: MODULE_H_ */