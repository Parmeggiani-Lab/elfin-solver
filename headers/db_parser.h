#ifndef DB_PARSER_H_
#define DB_PARSER_H_

#include "shorthands.h"
#include "radii.h"

#include "pair_relationship.h"

namespace elfin {
class DBParser
{
public:
    DBParser();
    ~DBParser();

    static void parse(
        const JSON & j,
        NameIdMap & name_map_out,
        IdNameMap & inm_out,
        RelaMat & rel_mat_out,
        RadiiList & radii_list_out);

};
}  /* elfin */

#endif  /* end of include guard: DB_PARSER_H_ */