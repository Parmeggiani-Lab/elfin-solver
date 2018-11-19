#include "database.h"

#include <unordered_map>

#include "string_types.h"
#include "random_utils.h"

#define DEBUG_PRINT_CMLPROBS

namespace elfin {

const Database * Database::instance_ = nullptr;

/* private */

void Database::distribute_cmlprobs() {
    size_t n_cmllink = 0, c_cmllink = 0;
    for (auto mod : mod_list_) {
        float n_cmlprob = (float) n_cmllink / link_total_;
        float c_cmlprob = (float) c_cmllink / link_total_;
        n_roulette.cmlprobs.push_back(n_cmlprob);
        c_roulette.cmlprobs.push_back(c_cmlprob);

#ifdef DEBUG_PRINT_CMLPROBS
        wrn("mod[%s] n_cmlprob=%.3f, c_cmlprob=%.3f\n",
            mod->name_.c_str(),
            n_roulette.cmlprobs.back(),
            c_roulette.cmlprobs.back());
#endif  /* ifdef DEBUG_PRINT_CMLPROBS */

        n_cmllink += mod->n_link_count();
        c_cmllink += mod->c_link_count();
    }

#ifdef DEBUG_PRINT_CMLPROBS
    wrn("Final n_cmlprob=%.3f, c_cmlprob=%.3f\n",
        (float) n_cmllink / link_total_,
        (float) c_cmllink / link_total_);
#endif  /* ifdef DEBUG_PRINT_CMLPROBS */
}

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
        mod_list_.push_back(new Module(name, mod_type, chain_ids));
    };

    for_each_module(init_module);

    link_total_ = 0;
    auto parse_link = [&](JSON_MOD_PARAMS) {
        Module * const mod_a = mod_list_.at(name_id_map[jit.key()]);

        // Add neighbouring nodes
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
                    const JSON & tx_json = xdb["n_to_c_tx"][tx_id];
                    Module::link_chains(
                        tx_json,
                        mod_a,
                        a_chain_id,
                        mod_b,
                        b_chain_it.key());
                    link_total_++;
                }
            }
        }
    };
    for_each_module(parse_link);

    distribute_cmlprobs();

    // Build radii list for collision checking
    auto parse_radii = [&](JSON_MOD_PARAMS) {
        const JSON & radii = (*jit)["radii"];
        const Module::Radii r = {
            radii["average_all"],
            radii["max_ca_dist"],
            radii["max_heavy_dist"]
        };
        mod_list_.at(name_id_map[jit.key()])->set_radii(r);
    };
    for_each_module(parse_radii);
}

Database::~Database() {
    for (auto mod : mod_list_) {
        delete mod;
    }
    mod_list_.clear();
}

}  /* elfin */