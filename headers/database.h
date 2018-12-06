/*
    Elfin's internal representation of the xdb.json database.
*/

#ifndef DATABASE_H_
#define DATABASE_H_

#include <vector>

#include "json.h"
#include "module.h"
#include "roulette.h"
#include "jutil.h"

#include <string>

namespace elfin {

// singleton class
class Database
{
protected:
    /* types */
    struct ModPtrRoulette : public Roulette<Module *> {
        std::string to_string() const;
    };

    /* data */
    ModPtrRoulette all_mods_; // This keeps the pointers to be freed
    ModPtrRoulette singles_, hubs_, basic_mods_, complex_mods_;

    /* modifiers */
    void categorize();

    /* printers */
    void print_roulettes();
    void print_db();
public:
    /* ctors */
    Database() {}

    /* dtors */
    virtual ~Database();

    /* getters */
    const ModPtrRoulette & all_mods() const { return all_mods_; }
    const ModPtrRoulette & singles() const { return singles_; }
    const ModPtrRoulette & hubs() const { return hubs_; }
    const ModPtrRoulette & basic_mods() const { return basic_mods_; }
    const ModPtrRoulette & complex_mods() const { return complex_mods_; }

    /* modifiers */
    void parse_from_json(const JSON & xdb);
};

}  /* elfin */

#endif  /* end of include guard: DATABASE_H_ */