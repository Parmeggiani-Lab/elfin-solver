/*
    Elfin's internal representation of the xdb.json database.
*/

#ifndef DATABASE_H_
#define DATABASE_H_

#include <vector>

#include "json.h"
#include "module.h"

namespace elfin {

typedef std::vector<Module *> ModuleList;

// singleton class
class Database
{
protected:
    /* data members */
    ModuleList mod_list_;
    static const Database * instance_ = nullptr;

    void init_probability_distribition() const;
public:
    /* ctors & dtors */
    Database(const JSON & j);
    virtual ~Database();

    /* getters & setters */
    const ModuleList & mod_list() const { return mod_list_; }
    static const Database * get_instance() { return instance_; }
};

extern const Database & XDB; // defined in db_parser.cc

}  /* elfin */

#endif  /* end of include guard: DATABASE_H_ */