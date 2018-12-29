#include "database.h"

#include <sstream>

#include "string_utils.h"
#include "random_utils.h"
#include "input_manager.h"
#include "debug_utils.h"

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
    mod_idx_map_.clear();
    singles_.clear();
    hubs_.clear();
    basic_mods_.clear();
    complex_mods_.clear();
}

//
// Divides modules in the following categories:
//   basic (2 termini)
//   complex (>2 termini)
//   singles
//   hubs
//
// Note: categorize() MUST be called after parsing links.
//
void Database::categorize() {
    for (auto& mod : all_mods_) {
        size_t const n_itf = mod->counts().all_interfaces();
        ProtoModule* mod_raw_ptr = mod.get();
        if (n_itf < 2) {
            PANIC("mod[%s] has fewer interfaces(%zu) than expected(2)\n",
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
            PANIC("mod[%s] has unknown ModuleType: %s\n",
                        mod->name.c_str(), ModuleTypeToCStr(mod->type));
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
                           k, n_links[k]->module_->name.c_str());
            }

            auto& c_links = proto_chain.c_term().links();
            for (size_t k = 0; k < c_links.size(); ++k)
            {
                JUtil.warn("\t\tc_links[%zu] -> xdb_[%s]\n",
                           k, c_links[k]->module_->name.c_str());
            }
        }
    }
}

/* public */
/* accessors */
ProtoModule const* Database::get_module(
    std::string const& name) const {
    auto itr = mod_idx_map_.find(name);

    ProtoModule const* mod = nullptr;

    if (itr != end(mod_idx_map_)) {
        mod = all_mods_.at(itr->second).get();
    }

    return mod;
}

/* modifiers */
#define JSON_MOD_PARAMS JSON::const_iterator jit, ModuleType mod_type
typedef std::function<void(JSON_MOD_PARAMS)> JsonModTypeLambda;
#define FOR_JSON(child, parent) \
    for(auto child = begin(parent); child != end(parent); ++child)

void Database::parse_from_json(JSON const& xdb) {
    reset();

    // Define lambas for code reuse
    JSON const& singles = xdb["modules"]["singles"];
    auto for_each_double_json = [&](JsonModTypeLambda const & lambda) {
        FOR_JSON (jit, singles) {
            lambda(jit, ModuleType::SINGLE);
        }
    };

    JSON const& hubs = xdb["modules"]["hubs"];
    auto for_each_hub_json = [&](JsonModTypeLambda const & lambda) {
        FOR_JSON (jit, hubs) {
            lambda(jit, ModuleType::HUB);
        }
    };

    auto for_each_module_json = [&](JsonModTypeLambda const & lambda) {
        for_each_double_json(lambda);
        for_each_hub_json(lambda);
    };

    // Non-finalized module list
    // Build mapping between name and id
    auto init_module = [&](JSON_MOD_PARAMS) {
        std::string const& name = jit.key();
        size_t const mod_id = all_mods_.size();
        mod_idx_map_[name] = mod_id;

#ifdef PRINT_MOD_IDX_MAP_
        JUtil.warn("Module %s maps to id %zu\n", name.c_str(), mod_id);
#endif  /* ifndef PRINT_MOD_IDX_MAP_ */

        StrList chain_names;
        JSON const& chains_json = (*jit)["chains"];
        FOR_JSON(chain_it, chains_json) {
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
        size_t const mod_a_id = mod_idx_map_[jit.key()];
        auto& mod_a = all_mods_.at(mod_a_id);

        JSON const& chains_json = (*jit)["chains"];
        FOR_JSON(a_chain_it, chains_json) {
            std::string const& a_chain_name = a_chain_it.key();

            // No need to run through "n" because xdb contains only n-c
            // transforms, i.e. "C-term extrusion" transforms.
            FOR_JSON(c_term_it, (*a_chain_it)["c"]) {
                size_t const mod_b_id = mod_idx_map_[c_term_it.key()];
                auto& mod_b = all_mods_.at(mod_b_id);

                FOR_JSON (b_chain_it, (*c_term_it)) {
                    size_t const tx_id = (*b_chain_it).get<size_t>();

                    // In create_proto_link(), an inversed version for c-n
                    // transform is created.
                    ProtoModule::create_proto_link_pair(
                        xdb,
                        tx_id,
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