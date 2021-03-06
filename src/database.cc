#include "database.h"

#include <sstream>

#include "string_utils.h"
#include "random_utils.h"
#include "debug_utils.h"
#include "options.h"
#include "json.h"

// #define PRINT_ROULETTES
// #define PRINT_DB
// #define PRINT_MOD_IDX_MAP_

namespace elfin {

/* protected */
void Database::ModPtrRoulette::print_to(std::ostream& os) const {
    for (size_t i = 0; i < items_.size(); ++i) {
        os << "Module";
        os << "[#" << i << ":" << items_.at(i)->name;
        os << "]=" << cpd_.at(i) << '\n';
    }
    os << "Total=" << total_ << '\n';
}

void Database::reset() {
    all_mods_.clear();
    ptterm_finders_.clear();
    mod_idx_map_.clear();
    singles_.clear();
    hubs_.clear();
    basic_mods_.clear();
    complex_mods_.clear();
    max_bp_degree_ = 0;
}

// Divides modules in the following categories:
//   basic (2 termini)
//   complex (>2 termini)
//   singles
//   hubs
//
// Note: categorize() MUST be called after parsing links.
void Database::categorize() {
    for (auto& mod : all_mods_) {
        size_t const n_itf = mod->counts().all_interfaces();
        auto mod_raw_ptr = mod.get();

        max_bp_degree_ = max(max_bp_degree_, n_itf);

        if (n_itf < 2) {
            if (mod->type == ModuleType::SYM_HUB) {
                JUtil.warn("Disabled symmetric hub \"%s\" did not parse interfaces\n",
                           mod->name.c_str());
                continue;
            }

            auto const& msg =
                string_format("mod[%s] has fewer interfaces(%zu) than expected(2)\n",
                              mod->name.c_str(), n_itf);
            PANIC(BadXDB(msg));
        } else if (n_itf == 2) {
            basic_mods_.push_back(mod->counts().all_links(), mod_raw_ptr);
        } else {
            complex_mods_.push_back(mod->counts().all_links(), mod_raw_ptr);
        }

        if (mod->type == ModuleType::SINGLE) {
            singles_.push_back(mod->counts().all_links(), mod_raw_ptr);
        } else if (mod->is_hub()) {
            hubs_.push_back(mod->counts().all_links(), mod_raw_ptr);
        } else {
            auto const& msg =
                string_format("mod[%s] has unknown ModuleType: %s\n",
                              mod->name.c_str(), ModuleTypeToCStr(mod->type));
            PANIC(BadXDB(msg));
        }
    }
}

void Database::print_roulettes() {
    std::ostringstream ss;
    ss << "---ProtoModule Roulettes Debug---\n";
    ss << "All:\n";
    for (auto& mod : all_mods_) {
        ss << mod->to_string();
    }
    ss << "Singles:\n" << singles_.to_string();
    ss << "Hubs:\n" << hubs_.to_string();
    ss << "Basic:\n" << basic_mods_.to_string();
    ss << "Complex:\n" << complex_mods_.to_string();
}

void Database::print_db() {
    JUtil.warn("---DB Proto Link Parse Debug---\n");
    size_t const n_mods = all_mods_.size();
    JUtil.warn("Database has %zu mods, of which...\n", n_mods);
    JUtil.warn("%zu are singles\n", singles_.items().size());
    JUtil.warn("%zu are hubs\n", hubs_.items().size());
    JUtil.warn("%zu are basic\n", basic_mods_.items().size());
    JUtil.warn("%zu are complex\n", complex_mods_.items().size());

    for (size_t i = 0; i < n_mods; ++i)
    {
        auto& mod = all_mods_.at(i);
        size_t const n_chains = mod->chains().size();
        JUtil.warn("xdb_[#%zu:%s] has %zu chains\n",
                   i, mod->name.c_str(), n_chains);

        for (auto& proto_chain : mod->chains()) {
            JUtil.warn("\tchain[#%zu:%s]:\n",
                       proto_chain.id,
                       proto_chain.name.c_str());

            auto& n_links = proto_chain.n_term().links();
            for (size_t k = 0; k < n_links.size(); ++k)
            {
                JUtil.warn("\t\tn_links[%zu] -> xdb_[%s]\n",
                           k, n_links[k]->module->name.c_str());
            }

            auto& c_links = proto_chain.c_term().links();
            for (size_t k = 0; k < c_links.size(); ++k)
            {
                JUtil.warn("\t\tc_links[%zu] -> xdb_[%s]\n",
                           k, c_links[k]->module->name.c_str());
            }
        }
    }
}

/* public */
/* accessors */
PtModKey Database::get_mod(std::string const& name) const {
    auto const itr = mod_idx_map_.find(name);

    if (itr == end(mod_idx_map_)) {
        throw ValueNotFound("Could not find " + name + " in XDB modules.");
    }

    return all_mods_.at(itr->second).get();
}

/* modifiers */
#define JSON_PARSER_PARAMS \
std::string const& key, JSON const& json, ModuleType mod_type

typedef std::function<void(JSON_PARSER_PARAMS)> JSONParser;

void Database::parse(Options const& options) {
    reset();

    JSON const& xdb = parse_json(options.xdb_file);

    // Define lambas for code reuse
    JSON const& singles = xdb.at("modules").at("singles");
    auto const for_each_double_json = [&](JSONParser const & lambda) {
        for (auto& [key, json] : singles.items()) {
            lambda(key, json, ModuleType::SINGLE);
        }
    };

    JSON const& hubs = xdb.at("modules").at("hubs");
    auto const for_each_hub_json = [&](JSONParser const & lambda) {
        for (auto& [key, json] : hubs.items()) {
            lambda(key, json, json.at("symmetric") == true ?
                   ModuleType::SYM_HUB : ModuleType::ASYM_HUB);
        }
    };

    auto for_each_module_json = [&](JSONParser const & lambda) {
        for_each_double_json(lambda);
        for_each_hub_json(lambda);
    };

    // Build mapping between name and id. This is not thread safe!
    auto const init_module = [&](JSON_PARSER_PARAMS) {
        size_t const mod_id = all_mods_.size();
        mod_idx_map_[key] = mod_id;

#ifdef PRINT_MOD_IDX_MAP_
        JUtil.warn("Module %s maps to id %zu\n", key.c_str(), mod_id);
#endif  /* ifndef PRINT_MOD_IDX_MAP_ */

        StrList chain_names;
        for (auto& [chain_name, json] : json.at("chains").items()) {
            chain_names.push_back(chain_name);
        }

        float const radius = ((float) json.at("radii")[options.radius_type]) * options.radius_factor;

        all_mods_.push_back(
            std::make_unique<ProtoModule>(
                key,
                mod_type,
                radius,
                chain_names));
    };
    for_each_module_json(init_module);

    auto const parse_link = [&](JSON_PARSER_PARAMS) {
        size_t const mod_a_id = mod_idx_map_[key];
        auto& mod_a = all_mods_.at(mod_a_id);

        for (auto& [a_chain_name, a_chain_json] : json.at("chains").items()) {
            // No need to run through "n" because xdb contains only n-c
            // transforms, i.e. "C-term extrusion" transforms.
            for (auto& [c_term_name, c_term_json] : a_chain_json.at("c").items()) {
                auto const& mod_b =
                    all_mods_.at(/*mod_b_id=*/mod_idx_map_[c_term_name]);

                for (auto& [b_chain_name, b_chain_json] : c_term_json.items()) {
                    if (mod_a->type == ModuleType::SYM_HUB or
                            mod_b->type == ModuleType::SYM_HUB) {
                        JUtil.warn("parse_link: symmetric hubs are currently disabled (%s to %s)\n",
                                   mod_a->name.c_str(), mod_b->name.c_str());
                        continue;
                    }

                    // In create_proto_link(), an inversed version for c-n
                    // transform is created.
                    ProtoModule::create_proto_link_pair(
                        xdb,
                        /*tx_id=*/b_chain_json.get<size_t>(),
                        *mod_a,
                        a_chain_name,
                        *mod_b,
                        b_chain_name);
                }
            }
        }
    };
    for_each_module_json(parse_link);

    // Finalize modules and add to all_mods_
    for (auto& mod : all_mods_) {
        mod->configure();
        for (auto& chain : mod->chains_) {
            size_t const chain_id = mod->get_chain_id(chain.name);
            if (not chain.n_term_.links().empty())
                ptterm_finders_.insert({mod.get(), chain_id, TermType::N, &chain.n_term_});
            if (not chain.c_term_.links().empty())
                ptterm_finders_.insert({mod.get(), chain_id, TermType::C, &chain.c_term_});
        }
    }

    categorize();

#ifdef PRINT_DB
    print_db();
#endif /* ifdef PRINT_DB */

#ifdef PRINT_ROULETTES
    print_roulettes();
#endif  /* ifdef PRINT_ROULETTES */
}

void Database::activate_ptterm_profile(PtTermFinderSet const& reachable) {
    for (auto const& finder : ptterm_finders_) {
        auto ptterm_ptr = finder.ptterm_ptr;
        if (reachable.find({nullptr, 0, TermType::NONE, ptterm_ptr}) == end(reachable)) {
            ptterm_ptr->deactivate();
        }
        else {
            ptterm_ptr->activate();
        }
    }

    for (auto& mod : all_mods_) {
        mod->configure();
    }
}


}  /* elfin */