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
private:
    /* data members */
    size_t link_total_;
    static const Database * instance_;

    /* other methods */
    void distribute_cmlprobs();

protected:
    /* data members */
    ModuleList mod_list_;
    struct {
        Roulette n, c, all;
        size_t n_total, c_total, all_total;
        void normalize() {
            n.normalize(n_total);
            c.normalize(c_total);
            all.normalize(all_total);
        }
    } roulettes_;

public:
    /* ctors & dtors */
    Database() {
        panic_if(instance_, "Multiple instantiation of Database()\n");
        instance_ = this;
    }
    virtual ~Database();
    void parse_from_json(const JSON & xdb);

    /* getters & setters */
    const ModuleList & mod_list() const { return mod_list_; }
    static const Database * instance() { return instance_; }
    Module const * get_rand_mod() const {
        return roulettes_.all.rand_item(mod_list_);
    }
    Module const * get_rand_mod_n() const {
        return roulettes_.n.rand_item(mod_list_);
    }
    Module const * get_rand_mod_c() const {
        return roulettes_.c.rand_item(mod_list_);
    }
};

extern const Database & XDB; // defined in db_parser.cc

}  /* elfin */

#endif  /* end of include guard: DATABASE_H_ */