#include "database.h"

#include <unordered_map>
#include <sstream>

#include "string_utils.h"
#include "random_utils.h"
#include "input_manager.h"
#include "debug_utils.h"

// #define PRINT_ROULETTES
// #define PRINT_DB
// #define PRINT_NAME_ID_MAP

namespace elfin {

/* protected */

std::string Database::ModPtrRoulette::to_string() const {
    std::ostringstream ss;
    for (size_t i = 0; i < items_.size(); ++i) {
        ss << "Module";
        ss << "[#" << i << ":" << items_.at(i)->name;
        ss << "]=" << cpd_.at(i) << '\n';
    }

    ss << "Total=" << total_ << '\n';
    return ss.str();
}

/*
 * Divides modules in the following categories:
 *   basic (2 termini)
 *   complex (>2 termini)
 *   singles
 *   hubs
 *
 * Note: Categorization must be done after link parsing.
 */
void Database::categorize() {
    for (auto mod : all_mods_.items()) {
        const size_t n_itf = mod->counts().interface;
        if (n_itf < 2) {
            die("mod[%s] has fewer interfaces(%lu) than expected(2)\n",
                mod->name.c_str(), n_itf);
        } else if (n_itf == 2) {
            basic_mods_.push_back(mod->counts().all_links(), mod);
        } else {
            complex_mods_.push_back(mod->counts().all_links(), mod);
        }

        if (mod->type == ModuleType::SINGLE) {
            singles_.push_back(mod->counts().all_links(), mod);
        } else if (mod->type == ModuleType::HUB) {
            hubs_.push_back(mod->counts().all_links(), mod);
        } else {
            die("mod[%s] has unknown ModuleType: %s\n",
                mod->name.c_str(), ModuleTypeNames[mod->type]);
        }
    }
}

void Database::print_roulettes() {
    wrn("---Module Roulettes Debug---\n");
    wrn("All:\n%s", all_mods_.to_string().c_str());
    wrn("Singles:\n%s", singles_.to_string().c_str());
    wrn("Hubs:\n%s", hubs_.to_string().c_str());
    wrn("Basic:\n%s", basic_mods_.to_string().c_str());
    wrn("Complex:\n%s", complex_mods_.to_string().c_str());
}

void Database::print_db() {
    wrn("---DB Link Parse Debug---\n");
    const size_t n_mods = all_mods_.items().size();
    wrn("Database has %lu mods, of which...\n", n_mods);
    wrn("%lu are singles\n", singles_.items().size());
    wrn("%lu are hubs\n", hubs_.items().size());
    wrn("%lu are basic\n", basic_mods_.items().size());
    wrn("%lu are complex\n", complex_mods_.items().size());

    for (size_t i = 0; i < n_mods; ++i)
    {
        const Module * mod = all_mods_.items().at(i);
        const size_t n_chains = mod->chains().size();
        wrn("xdb_[#%lu:%s] has %lu chains\n",
            i, mod->name.c_str(), n_chains);

        for (auto & chain : mod->chains()) {
            wrn("\tchain[#%lu:%s]:\n",
                mod->chain_id_map().at(chain.name),
                chain.name.c_str());

            const LinkList & n_links = chain.n_term().links();
            for (size_t k = 0; k < n_links.size(); ++k)
            {
                wrn("\t\tn_links[%lu] -> xdb_[%s]\n",
                    k, n_links[k].mod->name.c_str());
            }
            const LinkList & c_links = chain.c_term().links();
            for (size_t k = 0; k < c_links.size(); ++k)
            {
                wrn("\t\tc_links[%lu] -> xdb_[%s]\n",
                    k, c_links[k].mod->name.c_str());
            }
        }
    }
}

/* public */
#define JSON_MOD_PARAMS JSON::const_iterator jit, ModuleType mod_type
typedef std::function<void(JSON_MOD_PARAMS)> JsonModTypeLambda;

void Database::parse_from_json(const JSON & xdb) {
    // Define lambas for code reuse
    const JSON & singles = xdb["modules"]["singles"];
    auto for_each_double = [&](JsonModTypeLambda const & lambda) {
        for (JSON::const_iterator jit = singles.begin();
                jit != singles.end();
                ++jit) {
            lambda(jit, ModuleType::SINGLE);
        }
    };

    const JSON & hubs = xdb["modules"]["hubs"];
    auto for_each_hub = [&](JsonModTypeLambda const & lambda) {
        for (JSON::const_iterator jit = hubs.begin();
                jit != hubs.end();
                ++jit) {
            lambda(jit, ModuleType::HUB);
        }
    };

    auto for_each_module = [&](JsonModTypeLambda const & lambda) {
        for_each_double(lambda);
        for_each_hub(lambda);
    };

    // Non-finalized module list
    std::vector<Module *> nf_mod_list;

    // Build mapping between name and id
    std::unordered_map<std::string, size_t> name_id_map;
    auto init_module = [&](JSON_MOD_PARAMS) {
        const std::string & name = jit.key();
        const size_t mod_id = nf_mod_list.size();
        name_id_map[name] = mod_id;

#ifdef PRINT_NAME_ID_MAP
        wrn("Module %s maps to id %lu\n", name.c_str(), mod_id);
#endif  /* ifndef PRINT_NAME_ID_MAP */

        StrList chain_names;
        const JSON & chains_json = (*jit)["chains"];
        for (auto chain_it = chains_json.begin();
                chain_it != chains_json.end();
                ++chain_it) {
            chain_names.push_back(chain_it.key());
        }

        const float radius = (*jit)["radii"][OPTIONS.radius_type];
        nf_mod_list.push_back(new Module(name, mod_type, radius, chain_names));
    };
    for_each_module(init_module);

    auto parse_link = [&](JSON_MOD_PARAMS) {
        const size_t mod_a_id = name_id_map[jit.key()];
        Module * const mod_a =
            nf_mod_list.at(mod_a_id);

        const JSON & chains_json = (*jit)["chains"];
        for (auto a_chain_it = chains_json.begin();
                a_chain_it != chains_json.end();
                ++a_chain_it) {
            const std::string & a_chain_id = a_chain_it.key();

            // No need to run through "n" because xdb contains only n-c
            // transforms. In create_link(), an inversed version for c-n
            // transform is created.
            const JSON & c_json = (*a_chain_it)["c"];
            for (auto c_it = c_json.begin();
                    c_it != c_json.end();
                    ++c_it) {
                const size_t mod_b_id = name_id_map[c_it.key()];
                Module * const mod_b =
                    nf_mod_list.at(mod_b_id);

                const JSON & b_chains_json = (*c_it);
                for (auto b_chain_it = b_chains_json.begin();
                        b_chain_it != b_chains_json.end();
                        ++b_chain_it) {
                    const size_t tx_id = (*b_chain_it).get<size_t>();
                    NICE_PANIC(tx_id >= xdb["n_to_c_tx"].size(),
                               ("tx_id > xdb[\"n_to_c_tx\"].size()\n"
                                "\tEither xdb.json is corrupted or "
                                "there is an error in dbgen.py.\n"));

                    const JSON & tx_json = xdb["n_to_c_tx"][tx_id]["tx"];
                    Module::create_link(
                        tx_json,
                        mod_a,
                        a_chain_id,
                        mod_b,
                        b_chain_it.key());
                }
            }
        }
    };
    for_each_module(parse_link);

    // Finalize modules and add to all_mods_
    for (auto mod : nf_mod_list) {
        mod->finalize();
        all_mods_.push_back(mod->counts().all_links(), mod);
    }

    categorize();

#ifdef PRINT_DB
    print_db();
#endif /* ifdef PRINT_DB */

#ifdef PRINT_ROULETTES
    print_roulettes();
#endif  /* ifdef PRINT_ROULETTES */
}

Database::~Database() {
    for (auto mod : all_mods_.items()) {
        delete mod;
    }
}

}  /* elfin */