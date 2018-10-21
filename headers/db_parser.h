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
        NameIdMap & nameMapOut,
        IdNameMap & inmOut,
        RelaMat & relMatOut,
        RadiiList & radiiListOut);

};
}  /* elfin */

#endif  /* end of include guard: DB_PARSER_H_ */