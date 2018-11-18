#include "database.h"

#include "map_types.h"

namespace elfin {

/* protected */
void Database::init_probability_distribition() {

}

/* public */
Database::Database(const JSON & j) {
    /*
     * Database is a singleton class.
     */
    if (instance_) {
        die("Multiple initialization of Database singleton.\n");
    }

    // Define lambas for code reuse
    const JSON & double_data = j["double_data"];
    auto for_each_lambda = [&](auto lambda) {
        for (auto jit = double_data.begin();
                jit != double_data.end();
                ++jit) {
            lambda(jit);
        }
    };

    const JSON & hub_data = j["hub_data"];
    auto for_each_hub = [&](auto lambda) {
        for (auto jit = hub_data.begin();
                jit != hub_data.end();
                ++jit) {
            lambda(jit);
        }
    };

    auto for_each_module = [&](auto lambda) {
        for_each_double(lambda);
        for_each_hub(lambda);
    };

    StrIdMap name_id_map;
    auto add_name_id = [&](JSON::iterator jit) {
        const std::string & name = jit.key();
        mod_list_.push_back(nullptr);
        name_id_map[name] = mod_list_.size() - 1;
    };

    // Build mapping between name and id
    for_each_module(add_name_id);

    auto init_module = [&](
                           size_t id,
                           const std::string & name,
                           ModuleType type,
    const IdStrMap & chain_itn) {
        Module const * new_mod = mod_list_.at(id);
        if (!new_mod) {
            new_mod = new Module(name, type, chain_itn);
            mod_list_.push_back(new_mod);
        }
        return new_mod;
    };

    // Assuming a single module has only one chain
    IdStrMap single_chain_itn;
    single_chain_itn.push_back("A");

    auto parse_double_transform = [&](JSON::iterator jit) {
        const size_t id = jit;
        const JSON & double_json = *jit;
        Module * const single_a = init_module(
                                      id,
                                      jit.key(),
                                      ModuleType::SINGLE,
                                      single_chain_itn);

        // Add neighbouring nodes
        for (auto inner_it = double_json.begin();
                inner_it != double_json.end();
                ++inner_it) {
            const size_t inner_id = name_id_map[inner_it.key()];
            Module * const single_b = init_module(
                                          inner_id,
                                          inner_it.key(),
                                          ModuleType::SINGLE,
                                          single_chain_itn);
            Module::link_modules(
                *inner_it,
                single_a, 0
                single_b, 0);
        }
    };
    for_each_double(parse_double_transform);

    auto parse_hub_transform = [&](JSON::iterator jit) {
        const size_t id = jit;
        const JSON & hub_json = *jit;
        const JSON & comp_json = hub_json["component_data"];

        // Build hub chain id-to-name map
        IdStrMap hub_chain_itn;
        for (auto comp_it = comp_json.begin(),
                comp_it != comp_json.end();
                ++comp_it) {
            hub_chain_itn.push_back(comp_it.key());
        }
        Module * const hub = init_module(
                                 id,
                                 jit.key();
                                 ModuleType::HUB,
                                 hub_chain_itn);

        // Add neighbouring nodes
        for (auto inner_it = comp_json.begin();
                inner_it != comp_json.end();
                ++inner_it) {
            // Elfin assumes that no two hub directly interface with each
            // other. This means all modules listed under component_data must
            // have already been created by parse_double_transform. single_b
            // must therefore already exist.
            const size_t inner_id = name_id_map[inner_it.key()];
            Module * const single_b = mod_list_.at(inner_id);
            if (!single_b) {
                die("Module not found during add_hub_transform()\n");
            }

            Module::link_modules(
                *inner_it,
                hub, 0
                single_b, 0);
        }
    };
    for_each_hub(parse_hub_transform);

    init_probability_distribition();

    instance_ = this; // set singleton instance
}

Database::~Database() {
    for (Modue * mod : mod_list_) {
        delete mod;
    }
    mod_list_.clear();
}

}  /* elfin */