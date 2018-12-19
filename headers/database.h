/*
    Elfin's internal representation of the xdb.json database.
*/

#ifndef DATABASE_H_
#define DATABASE_H_

#include <vector>

#include "json.h"
#include "proto_module.h"
#include "roulette.h"
#include "vector_utils.h"

#include <string>

namespace elfin {

// singleton class
class Database
{
protected:
    /* types */
    struct ModPtrRoulette : public Roulette<ProtoModule *> {
        std::string to_string() const;
    };

    /* data */
    Vector<ProtoModuleUP> all_mods_;
    ModPtrRoulette singles_, hubs_, basic_mods_, complex_mods_;

    /* modifiers */
    void reset();
    void categorize();

    /* printers */
    void print_roulettes();
    void print_db();
public:
    /* getters */
    ModPtrRoulette const& singles() const { return singles_; }
    ModPtrRoulette const& hubs() const { return hubs_; }
    ModPtrRoulette const& basic_mods() const { return basic_mods_; }
    ModPtrRoulette const& complex_mods() const { return complex_mods_; }

    /* modifiers */
    void parse_from_json(JSON const& xdb);
};

}  /* elfin */

#endif  /* end of include guard: DATABASE_H_ */