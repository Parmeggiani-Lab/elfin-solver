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
void Database::reset() {
    all_mods_.clear();
    singles_.clear();
    hubs_.clear();
    basic_mods_.clear();
    complex_mods_.clear();
}

void Database::categorize() {
    for (auto& mod : all_mods_) {
        size_t const n_itf = mod->counts().all_interfaces();
        ProtoModule* mod_raw_ptr = mod.get();
        if (n_itf < 2) {
            die("mod[%s] has fewer interfaces(%lu) than expected(2)\n",
                mod->name.c_str(), n_itf);
        } else if (n_itf == 2) {
            basic_mods_.push_back(mod->counts().all_links(), mod_raw_ptr);
        } else {
            complex_mods_.push_back(mod->counts().all_links(), mod_raw_ptr);
        }

        if (mod->type == ModuleType::SINGLE) {
            singles_.push_back(mod->counts().all_links(), mod_raw_ptr);
        } else if (mod->type == ModuleType::HUB) {
            hubs_.push_back(mod->counts().all_links(), mod_raw_ptr);
        } else {
            die("mod[%s] has unknown ModuleType: %s\n",
                mod->name.c_str(), ModuleTypeToCStr(mod->type));
        }
    }
}

void Database::print_roulettes() {
    wrn("---ProtoModule Roulettes Debug---\n");
    wrn("All:\n");
    for (auto& mod : all_mods_) {
        raw_at(LOG_WARN, mod->to_string().c_str());
    }
    wrn("Singles:\n%s", singles_.to_string().c_str());
    wrn("Hubs:\n%s", hubs_.to_string().c_str());
    wrn("Basic:\n%s", basic_mods_.to_string().c_str());
    wrn("Complex:\n%s", complex_mods_.to_string().c_str());
}

void Database::print_db() {
    wrn("---DB Proto Link Parse Debug---\n");
    size_t const n_mods = all_mods_.size();
    wrn("Database has %lu mods, of which...\n", n_mods);
    wrn("%lu are singles\n", singles_.items().size());
    wrn("%lu are hubs\n", hubs_.items().size());
    wrn("%lu are basic\n", basic_mods_.items().size());
    wrn("%lu are complex\n", complex_mods_.items().size());

    for (size_t i = 0; i < n_mods; ++i)
    {
        auto& mod = all_mods_.at(i);
        size_t const n_chains = mod->chains().size();
        wrn("xdb_[#%lu:%s] has %lu chains\n",
            i, mod->name.c_str(), n_chains);

        for (auto& proto_chain : mod->chains()) {
            wrn("\tchain[#%lu:%s]:\n",
                proto_chain.id,
                proto_chain.name.c_str());

            auto& n_links = proto_chain.n_term().links();
            for (size_t k = 0; k < n_links.size(); ++k)
            {
                wrn("\t\tn_links[%lu] -> xdb_[%s]\n",
                    k, n_links[k]->module_->name.c_str());
            }

            auto& c_links = proto_chain.c_term().links();
            for (size_t k = 0; k < c_links.size(); ++k)
            {
                wrn("\t\tc_links[%lu] -> xdb_[%s]\n",
                    k, c_links[k]->module_->name.c_str());
            }
        }
    }
}

/* public */
#define JSON_MOD_PARAMS JSON::const_iterator jit, ModuleType mod_type
typedef std::function<void(JSON_MOD_PARAMS)> JsonModTypeLambda;

void Database::parse_from_json(JSON const& xdb) {
    reset();

    // Define lambas for code reuse
    JSON const& singles = xdb["modules"]["singles"];
    auto for_each_double_json = [&](JsonModTypeLambda const & lambda) {
        for (JSON::const_iterator jit = singles.begin();
                jit != singles.end();
                ++jit) {
            lambda(jit, ModuleType::SINGLE);
        }
    };

    JSON const& hubs = xdb["modules"]["hubs"];
    auto for_each_hub_json = [&](JsonModTypeLambda const & lambda) {
        for (JSON::const_iterator jit = hubs.begin();
                jit != hubs.end();
                ++jit) {
            lambda(jit, ModuleType::HUB);
        }
    };

    auto for_each_module_json = [&](JsonModTypeLambda const & lambda) {
        for_each_double_json(lambda);
        for_each_hub_json(lambda);
    };

    // Non-finalized module list
    // Build mapping between name and id
    std::unordered_map<std::string, size_t> name_id_map;
    auto init_module = [&](JSON_MOD_PARAMS) {
        const std::string& name = jit.key();
        size_t const mod_id = all_mods_.size();
        name_id_map[name] = mod_id;

#ifdef PRINT_NAME_ID_MAP
        wrn("Module %s maps to id %lu\n", name.c_str(), mod_id);
#endif  /* ifndef PRINT_NAME_ID_MAP */

        StrList chain_names;
        JSON const& chains_json = (*jit)["chains"];
        for (auto chain_it = chains_json.begin();
                chain_it != chains_json.end();
                ++chain_it) {
            chain_names.push_back(chain_it.key());
        }

        float const radius = (*jit)["radii"][OPTIONS.radius_type];
        all_mods_.push_back(
            std::make_unique<ProtoModule>(
                name,
                mod_type,
                radius,
                chain_names));
    };
    for_each_module_json(init_module);

    auto parse_link = [&](JSON_MOD_PARAMS) {
        size_t const mod_a_id = name_id_map[jit.key()];
        auto& mod_a = all_mods_.at(mod_a_id);

        JSON const& chains_json = (*jit)["chains"];
        for (auto a_chain_it = chains_json.begin();
                a_chain_it != chains_json.end();
                ++a_chain_it) {
            const std::string& a_chain_name = a_chain_it.key();

            // No need to run through "n" because xdb contains only n-c
            // transforms. In create_proto_link(), an inversed version for c-n
            // transform is created.
            JSON const& c_json = (*a_chain_it)["c"];
            for (auto c_it = c_json.begin();
                    c_it != c_json.end();
                    ++c_it) {
                size_t const mod_b_id = name_id_map[c_it.key()];
                auto& mod_b = all_mods_.at(mod_b_id);

                JSON const& b_chains_json = (*c_it);
                for (auto b_chain_it = b_chains_json.begin();
                        b_chain_it != b_chains_json.end();
                        ++b_chain_it) {
                    size_t const tx_id = (*b_chain_it).get<size_t>();
                    NICE_PANIC(tx_id >= xdb["n_to_c_tx"].size(),
                               ("tx_id > xdb[\"n_to_c_tx\"].size()\n"
                                "\tEither xdb.json is corrupted or "
                                "there is an error in dbgen.py.\n"));

                    JSON const& tx_json = xdb["n_to_c_tx"][tx_id];
                    ProtoModule::create_proto_link_pair(
                        tx_json,
                        *mod_a,
                        a_chain_name,
                        *mod_b,
                        b_chain_it.key());
                }
            }
        }
    };
    for_each_module_json(parse_link);

    // Finalize modules and add to all_mods_
    for (auto& mod : all_mods_) {
        mod->finalize();
    }

    categorize();

#ifdef PRINT_DB
    print_db();
#endif /* ifdef PRINT_DB */

#ifdef PRINT_ROULETTES
    print_roulettes();
#endif  /* ifdef PRINT_ROULETTES */
}

}  /* elfin */