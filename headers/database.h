/*
    Elfin's internal representation of the xdb.json database.
*/

#ifndef DATABASE_H_
#define DATABASE_H_

#include <string>
#include <vector>

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

    /* data */
    std::vector<ProtoModuleSP> all_mods_;
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
    ModPtrRoulette const& singles() const { return singles_; }
    ModPtrRoulette const& hubs() const { return hubs_; }
    ModPtrRoulette const& basic_mods() const { return basic_mods_; }
    ModPtrRoulette const& complex_mods() const { return complex_mods_; }
    ProtoModule const* get_module(std::string const& name) const;

    /* modifiers */
    void parse(Options const& options);
};

}  /* elfin */

#endif  /* end of include guard: DATABASE_H_ */