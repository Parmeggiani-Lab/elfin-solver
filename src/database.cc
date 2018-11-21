#include "database.h"

#include <unordered_map>

#include "string_types.h"
#include "random_utils.h"
#include "options.h"

// #define DEBUG_PRINT_CMLPROBS

namespace elfin {

const Database * Database::instance_ = nullptr;

/* private */

/* public */
#define JSON_MOD_PARAMS JSON::const_iterator jit, ModuleType mod_type
typedef std::function<void(JSON_MOD_PARAMS)> JsonModTypeLambda;
void Database::parse_from_json(const JSON & xdb) {
    mod_list_.clear();

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

    // Build mapping between name and id
    std::unordered_map<std::string, size_t> name_id_map;
    auto init_module = [&](JSON_MOD_PARAMS) {
        const std::string & name = jit.key();
        const size_t mod_id = mod_list_.size();
        name_id_map[name] = mod_id;

        StrList chain_ids;
        const JSON & chains_json = (*jit)["chains"];
        for (auto chain_it = chains_json.begin();
                chain_it != chains_json.end();
                ++chain_it) {
            chain_ids.push_back(chain_it.key());
        }

        const float radius = (*jit)["radii"][OPTIONS.radius_type];
        mod_list_.push_back(new Module(name, mod_type, radius, chain_ids));
    };
    for_each_module(init_module);

    size_t link_total = 0;
    auto parse_link = [&](JSON_MOD_PARAMS) {
        Module * const mod_a = mod_list_.at(name_id_map[jit.key()]);

        const JSON & chains_json = (*jit)["chains"];
        for (auto a_chain_it = chains_json.begin();
                a_chain_it != chains_json.end();
                ++a_chain_it) {
            const std::string & a_chain_id = a_chain_it.key();

            // No need to run through "n" because xdb contains only n-c
            // transforms. In link_chains we create inversed versions for c-n
            // transforms.

            const JSON & c_json = (*a_chain_it)["c"];
            for (auto c_it = c_json.begin();
                    c_it != c_json.end();
                    ++c_it) {
                Module * const mod_b = mod_list_.at(name_id_map[c_it.key()]);

                const JSON & b_chains_json = (*c_it);
                for (auto b_chain_it = b_chains_json.begin();
                        b_chain_it != b_chains_json.end();
                        ++b_chain_it) {
                    const size_t tx_id = (*b_chain_it).get<size_t>();
                    panic_if(tx_id >= xdb["n_to_c_tx"].size(),
                             ("tx_id > xdb[\"n_to_c_tx\"].size()\n"
                              "\tEither xdb.json is corrupted or "
                              "there is a coding error in dbgen.py...\n"));

                    const JSON & tx_json = xdb["n_to_c_tx"][tx_id]["tx"];
                    Module::link_chains(
                        tx_json,
                        mod_a,
                        a_chain_id,
                        mod_b,
                        b_chain_it.key());

                    link_total++;
                }
            }
        }
    };
    for_each_module(parse_link);

    size_t n_acm = 0, c_acm = 0, all_acm = 0;
    for (auto mod : mod_list_) {
        roulettes_.n.cmlprobs.push_back(
            (float) n_acm / link_total);
        roulettes_.c.cmlprobs.push_back(
            (float) c_acm / link_total);
        roulettes_.all.cmlprobs.push_back(
            (float) all_acm / (2 * link_total));
        n_acm += mod->n_link_count();
        c_acm += mod->c_link_count();
        all_acm += mod->all_link_count();
    }

#ifdef DEBUG_PRINT_CMLPROBS
    wrn("---Cumulative Probability Debug---\n");
    wrn("Total %lu links\n", link_total);
    for (size_t i = 0; i < mod_list_.size(); ++i) {
        wrn("mod[#%lu:%s] n=%.3f, c=%.3f, all=%.3f\n",
            i, mod_list_.at(i)->name_.c_str(),
            roulettes_.n.cmlprobs.at(i),
            roulettes_.c.cmlprobs.at(i),
            roulettes_.all.cmlprobs.at(i));
    }
#endif  /* ifdef DEBUG_PRINT_CMLPROBS */
}

Database::~Database() {
    for (auto mod : mod_list_) {
        delete mod;
    }
    mod_list_.clear();
}

}  /* elfin */