#include "db_parser.h"

#include "database.h"
#include "jutil.h"

#define PRINT_PARSED_DB

namespace elfin {

Database xdb_;
const Database & XDB = xdb_;

void DBParser::parse(const JSON & j) {
    xdb_ = Database::parse_from_json(j);

#ifdef PRINT_PARSED_DB
    const size_t n_mods = xdb_.mod_list().size();
    wrn("Database has %lu mods\n", n_mods);

    for (size_t i = 0; i < n_mods; ++i)
    {
        const size_t n_chains = mod.chains().size();
        wrn("xdb_[#%lu:%s] has %lu chains\n",
            i, mod.name_.c_str(), n_chains);

        for (size_t j = 0; j < n_chains; ++j)
        {
            wrn("\tchain[#%lu:%s]:\n",
                j, mod.chain_itn_[j].c_str());

            const Chain & chain = mod.chains()[j];
            auto n_links = chain.n_links;
            for (size_t k = 0; k < n_links.size(); ++k)
            {
                wrn("\t\tn_links[%lu] -> xdb_[%s]\n", 
                    k, std::get<1>(n_links[k])->name_.c_str());
            }
            auto c_links = chain.c_links;
            for (size_t k = 0; k < c_links.size(); ++k)
            {
                wrn("\t\tc_links[%lu] -> xdb_[%s]\n", 
                    k, std::get<1>(c_links[k])->name_.c_str());
            }
        }
    }
#endif /* ifdef PRINT_PARSED_DB */
}

}  /* elfin */