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

namespace elfin {

// singleton class
class Database
{
protected:
    /* types */
    typedef std::vector<Module *> ModPtrList;
    typedef Roulette<ModPtrList, Module *> ModPtrRoulette;

    /* data members */
    struct Drawable {
        ModPtrList mod_list;
        ModPtrRoulette all;
        ModPtrRoulette n;
        ModPtrRoulette c;
        std::string to_string() const;
        void init_cml_sums();
        Drawable() : all(mod_list), n(mod_list), c(mod_list) {}
        Drawable & operator=(const Drawable & other) {
            mod_list = other.mod_list; // copy mod_list
            // Don't copy Rouletes because they rely on ref to this->mod_list
            return *this;
        }
    };

    struct Drawables {
        /*
         * Basic: mods with 2 termini
         * Complex: mods with > 2 termini
         */
        Drawable all_mods; // This keeps the pointers to be freed
        Drawable singles, hubs, basic, complex;
        void categorize();
        void init_cml_sums() {
            all_mods.init_cml_sums();
            singles.init_cml_sums();
            hubs.init_cml_sums();
            basic.init_cml_sums();
            complex.init_cml_sums();
        }
    };

    Drawables drawables_;

    /* other methods */
    void print_drawables();
    void print_db();
public:
    /* ctors & dtors */
    Database() {}
    virtual ~Database();

    const Drawables & get_drawables() const { return drawables_; }

    /* other methods */
    void parse_from_json(const JSON & xdb);
};

}  /* elfin */

#endif  /* end of include guard: DATABASE_H_ */