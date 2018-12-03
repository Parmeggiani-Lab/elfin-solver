#include "database.h"

#include <unordered_map>
#include <sstream>

#include "string_utils.h"
#include "random_utils.h"
#include "input_manager.h"

// #define PRINT_DRAWABLES
#define PRINT_DB
#define PRINT_NAME_ID_MAP

namespace elfin {

void Database::Drawable::init_cml_sums() {
    for (auto fin_mod : mod_list) {
        all.cumulate(fin_mod->counts.all_link());
        n.cumulate(fin_mod->counts.n_link);
        c.cumulate(fin_mod->counts.c_link);
    }
}

std::string Database::Drawable::to_string() const {
    std::stringstream ss;
    for (size_t i = 0; i < mod_list.size(); ++i) {
        ss << string_format(
               "mod[#%lu:%s] n=%lu, c=%lu, all=%lu\n",
               i,
               mod_list.at(i)->name.c_str(),
               n.cml_sum().at(i),
               c.cml_sum().at(i),
               all.cml_sum().at(i));
    }

    ss << string_format("Total: n=%lu, c=%lu, all=%lu\n",
                        n.total(),
                        c.total(),
                        all.total());
    return ss.str();
}

/*
 * Divides modules in the following categories:
 *   basic (2 termini)
 *   complex (>2 termini)
 *   singles
 *   hubs
 *
 * Note: Categorization can only be done after link parsing.
 */
void Database::Drawables::categorize() {
    for (auto mod : all_mods.mod_list) {
        const size_t n_itf = mod->counts.interface;
        if (n_itf < 2) {
            die("mod[%s] has fewer interfaces(%lu) than expected(2)\n",
                mod->name.c_str(), n_itf);
        } else if (n_itf == 2) {
            basic.mod_list.push_back(mod);
        } else {
            complex.mod_list.push_back(mod);
        }

        if (mod->type == ModuleType::SINGLE) {
            singles.mod_list.push_back(mod);
        } else if (mod->type == ModuleType::HUB) {
            hubs.mod_list.push_back(mod);
        } else {
            die("mod[%s] has unknown ModuleType: %s\n",
                mod->name.c_str(), ModuleTypeNames[mod->type]);
        }
    }
}

/* protected */
void Database::print_drawables() {
#ifdef PRINT_DRAWABLES
    wrn("---Drawables Debug---\n");
    wrn("All:\n%s", drawables_.all_mods.to_string().c_str());
    wrn("Singles:\n%s", drawables_.singles.to_string().c_str());
    wrn("Hubs:\n%s", drawables_.hubs.to_string().c_str());
    wrn("Basic:\n%s", drawables_.basic.to_string().c_str());
    wrn("Complex:\n%s", drawables_.complex.to_string().c_str());
#endif  /* ifdef PRINT_DRAWABLES */
}

void Database::print_db() {
#ifdef PRINT_DB
    wrn("---DB Link Parse Debug---\n");
    const size_t n_mods = drawables_.all_mods.mod_list.size();
    wrn("Database has %lu mods, of which...\n", n_mods);
    wrn("%lu are singles\n", drawables_.singles.mod_list.size());
    wrn("%lu are hubs\n", drawables_.hubs.mod_list.size());
    wrn("%lu are basic\n", drawables_.basic.mod_list.size());
    wrn("%lu are complex\n", drawables_.complex.mod_list.size());

    for (size_t i = 0; i < n_mods; ++i)
    {
        const Module * mod = drawables_.all_mods.mod_list.at(i);
        const size_t n_chains = mod->chains.size();
        wrn("xdb_[#%lu:%s] has %lu chains\n",
            i, mod->name.c_str(), n_chains);

        for (auto & chain : mod->chains) {
            wrn("\tchain[#%lu:%s]:\n",
                mod->chain_id_map.at(chain.name),
                chain.name.c_str());

            auto & n_links = chain.n_term.links;
            for (size_t k = 0; k < n_links.size(); ++k)
            {
                wrn("\t\tn_links[%lu] -> xdb_[%s]\n",
                    k, n_links[k].mod->name.c_str());
            }
            auto & c_links = chain.n_term.links;
            for (size_t k = 0; k < c_links.size(); ++k)
            {
                wrn("\t\tc_links[%lu] -> xdb_[%s]\n",
                    k, c_links[k].mod->name.c_str());
            }
        }
    }
#endif /* ifdef PRINT_DB */
}

/* public */
#define JSON_MOD_PARAMS JSON::const_iterator jit, ModuleType mod_type
typedef std::function<void(JSON_MOD_PARAMS)> JsonModTypeLambda;

void Database::parse_from_json(const JSON & xdb) {
    drawables_ = Drawables();

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
    ModPtrList & nf_mod_list = drawables_.all_mods.mod_list;

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
        err("mod_a_id=%lu\n", mod_a_id);

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
                err("mod_b_id=%lu\n", mod_b_id);

                const JSON & b_chains_json = (*c_it);
                for (auto b_chain_it = b_chains_json.begin();
                        b_chain_it != b_chains_json.end();
                        ++b_chain_it) {
                    const size_t tx_id = (*b_chain_it).get<size_t>();
                    panic_if(tx_id >= xdb["n_to_c_tx"].size(),
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

    // Finalize modules
    // for (auto mod : nf_mod_list) {
    //     mod->finalize();
    // }

    drawables_.categorize();
    drawables_.init_cml_sums();

    print_db();
    print_drawables();
}

Database::~Database() {
    for (auto mod : drawables_.all_mods.mod_list) {
        delete mod;
    }
}

}  /* elfin */