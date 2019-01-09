/*
    Elfin's internal representation of the xdb.json database.
*/

#ifndef DATABASE_H_
#define DATABASE_H_

#include <string>
#include <vector>
#include <unordered_set>

#include "json.h"
#include "proto_module.h"
#include "roulette.h"

namespace elfin {

/* Fwd Decl */
struct Options;

class Database {
protected:
    /* types */
    struct ModPtrRoulette :
        public Roulette<ProtoModule *>, public Printable {
        virtual void print_to(std::ostream& os) const;
    };

    struct PtTermFinder {
        /* types */
        struct hasher {
            size_t operator()(PtTermFinder const& f) const {
                return std::hash<ProtoTerm*>()(f.ptterm_ptr);
            }
        };
        struct comparer {
            size_t operator()(PtTermFinder const& lhs, PtTermFinder const& rhs) const {
                return lhs.ptterm_ptr == rhs.ptterm_ptr;
            }
        };

        /* data */
        PtModKey mod;
        size_t chain_id;
        TermType term;
        ProtoTerm* ptterm_ptr;
    };
    typedef std::unordered_set <PtTermFinder, PtTermFinder::hasher, PtTermFinder::comparer> PtTermFinders;

    /* data */
    std::vector<ProtoModuleSP> all_mods_;
    PtTermFinders ptterm_finders_;
    StrIndexMap mod_idx_map_;
    ModPtrRoulette singles_, hubs_, basic_mods_, complex_mods_;

    /* modifiers */
    void reset();
    void categorize();

    /* printers */
    void print_roulettes();
    void print_db();
public:
    /* accessors */
    std::vector<ProtoModuleSP> const& all_mods() const { return all_mods_; }
    PtTermFinders const& ptterm_finders() const { return ptterm_finders_; }
    StrIndexMap const& mod_idx_map() const { return mod_idx_map_; }
    ModPtrRoulette const& singles() const { return singles_; }
    ModPtrRoulette const& hubs() const { return hubs_; }
    ModPtrRoulette const& basic_mods() const { return basic_mods_; }
    ModPtrRoulette const& complex_mods() const { return complex_mods_; }
    PtModKey get_mod(std::string const& name) const;

    /* modifiers */
    void parse(Options const& options);
    void activate_ptterm_profile(PtTermKeySet const& reachable);
    void deactivate_ptterm_profile();
};

}  /* elfin */

#endif  /* end of include guard: DATABASE_H_ */