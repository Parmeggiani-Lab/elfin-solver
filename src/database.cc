#include "database.h"

#include <unordered_map>

#include "string_types.h"
#include "random_utils.h"

#define DEBUG_PRINT_CMLPROBS

namespace elfin {

/* private */
void Database::parse_from_json(const JSON & j) {
    // Define lambas for code reuse
    const JSON & double_data = j["double_data"];
    auto for_each_double = [&](auto lambda) {
        for (auto jit = double_data.begin();
                jit != double_data.end();
                ++jit) {
            lambda(jit, ModuleType::SINGLE);
        }
    };

    const JSON & hub_data = j["hub_data"];
    auto for_each_hub = [&](auto lambda) {
        for (auto jit = hub_data.begin();
                jit != hub_data.end();
                ++jit) {
            lambda(jit, ModuleType::HUB);
        }
    };

    auto for_each_module = [&](auto lambda) {
        for_each_double(lambda);
        for_each_hub(lambda);
    };

    // Build mapping between name and id
    std::unordered_map<std::string, size_t> name_id_map;
    auto init_module = [&](JSON::iterator & jit, ModuleType mod_type) {
        const std::string & name = jit.key();
        const size_t mod_id = mod_list_.size();
        name_id_map[name] = mod_id;

        StrList chain_names;
        const JSON & comp_json = (*jit)["component_data"];
        for (auto & it : comp_json) {
            chain_names.push_back(it.first);
        }
        mod_list_.push_back(new Module(name, mod_type, chain_names));
    };

    for_each_module(init_module);

    link_total_ = 0;
    auto parse_link = [&](JSON::iterator & jit) {
        const std::string & mod_name = jit.key();
        const size_t mod_id = name_id_map[mod_name];
        const Module * mod_a = mod_list_.at(mod_id);

        // Add neighbouring nodes
        const JSON & comp_json = (*jit)["component_data"];
        for (auto & it : comp_json) {
            const std::string & mod_b_name = it.first;
            const size_t mod_b_id = name_id_map[mod_b_name];
            Module * const mod_b = mod_list_.at(mod_b_id);

            Module::link_chains(
                it.second,
                mod_a->chains().at(mod_b_name),
                mod_b->chains().at(single_chain_name));
            link_total_++;
        }
    };
    for_each_module(parse_link);

    distribute_cmlprobs();

    // Build radii list for collision checking
    const JSON & single_data = j["single_data"];
    for (auto single_it = single_data.begin();
            single_it != single_data.end();
            ++single_it)
    {
        const JSON & radii = (*single_it)["radii"];
        const Radii r = {
            radii["average_all"],
            radii["max_ca_dist"],
            radii["max_heavy_dist"]
        };
        mod_list_[name_id_map[single_it.key()]].set_radii(r);
    }
}

void Database::distribute_cmlprobs() {
    size_t n_cmllink = 0, c_cmllink = 0;
    for (auto mod : mod_list_) {
        float n_cmlprob = (float) n_cmllink / link_total_;
        float c_cmlprob = (float) c_cmllink / link_total_;
        n_roulette.push_back(n_cmlprob);
        c_roulette.push_back(c_cmlprob);

#ifdef DEBUG_PRINT_CMLPROBS
        wrn("mod[%s] n_cmlprob=%.3f, c_cmlprob=%.3f\n",
            mod->name_.c_str(), n_roulette.back(), c_roulette.back());
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
Database::Database(const JSON & j) {
    /*
     * Database is a singleton class.
     */
    if (instance_) {
        die("Multiple initialization of Database singleton.\n");
    }

    parse_from_json(j);

    instance_ = this; // set singleton instance
}

Database::~Database() {
    for (Modue * mod : mod_list_) {
        delete mod;
    }
    mod_list_.clear();
}

}  /* elfin */