#ifndef DB_PARSER_H_
#define DB_PARSER_H_

#include "json.h"

namespace elfin {
class DBParser
{
public:
    DBParser();
    ~DBParser();

    static void parse(const JSON & j);
};
}  /* elfin */

#endif  /* end of include guard: DB_PARSER_H_ */