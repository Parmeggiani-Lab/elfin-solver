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

typedef std::vector<Module *> ModuleList;

// singleton class
class Database
{
protected:
    /* data members */
    struct Drawable {
        ModuleList mod_list;
        Roulette n, c, all;
        void compute_cmlprobs();
        Module const * draw_all() const {
            return all.rand_item(mod_list);
        }
        Module const * draw_n() const {
            return n.rand_item(mod_list);
        }
        Module const * draw_c() const {
            return c.rand_item(mod_list);
        }
        std::string to_string() const;
    };

    struct Drawables {
        /*
         * Basic: mods with 2 termini
         * Complex: mods with > 2 termini
         */
        Drawable all; // This keeps the pointers to be freed 
        Drawable singles, hubs, basic, complex;
        void compute_cmlprobs() {
            all.compute_cmlprobs();
            singles.compute_cmlprobs();
            hubs.compute_cmlprobs();
            basic.compute_cmlprobs();
            complex.compute_cmlprobs();
        }

        void categorize();
    };

    Drawables drawables_;

    /* other methods */
    void print_cmlprobs();
    void print_db();
public:
    /* ctors & dtors */
    Database() {}
    virtual ~Database();

    /* other methods */
    void parse_from_json(const JSON & xdb);
};

}  /* elfin */

#endif  /* end of include guard: DATABASE_H_ */