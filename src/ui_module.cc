#include "ui_module.h"
#include "input_manager.h"

namespace elfin {

std::string parse_module_name(JSON const& json) {
    std::string res;
    try {
        res = json.at("module_name");
    } catch (JSON::exception const& je) {
        JSON_LOG_EXIT(je);
    }
    return res;
}

UIModule::Linkage parse_linkage(JSON const& json) {
    UIModule::Linkage res;

    try {
        auto parse = [&](std::string const & linkage_name) {
            for (auto const& link_json : json[linkage_name]) {
                std::string const& term_str = link_json.at("terminus");

                if (VALID_TERM_NAMES.find(term_str) == end(VALID_TERM_NAMES)) {
                    throw CouldNotParse("Unknown terminus name: " + term_str + ".");
                }

                res.push_back({
                    parse_term(term_str),
                    link_json.at("source_chain_id"),
                    link_json.at("target_chain_id"),
                    link_json.at("target_mod")
                });
            }
        };

        parse("n_linkage");
        parse("c_linkage");
    } catch (JSON::exception const& je) {
        JSON_LOG_EXIT(je);
    }

    return res;
}

PtModKey parse_prototype(std::string const& module_name) {
    return XDB.get_mod(module_name);
}

FreeTerms parse_free_terms(PtModKey const mod, UIModule::Linkage const& linkage) {
    std::vector<FreeTerm> busy_fterms;
    for (const auto& link : linkage) {
        busy_fterms.push_back(FreeTerm(nullptr, mod->get_chain_id(link.src_chain_name), link.term));
    }

    FreeTerms res;
    for (auto const& ft : mod->free_terms()) {
        auto const is_ft = [&](FreeTerm const & bft) { return ft.nodeless_compare(bft); };
        if (std::find_if(begin(busy_fterms), end(busy_fterms), is_ft) == end(busy_fterms)) {
            res.push_back(ft);
        }
    }
    return res;
}

std::vector<PtTermKey> parse_busy_ptterms(PtModKey const mod, FreeTerms const& free_terms) {
    std::vector<PtTermKey> res;
    for (const auto& ft : free_terms) {
        res.push_back(&(mod->get_term(ft)));
    }
    return res;
}

/* ctors */
UIModule::UIModule(std::string const& name,
                   JSON const& json) :
    UIObject(name, json),
    module_name(parse_module_name(json)),
    prototype(parse_prototype(module_name)),
    linkage(parse_linkage(json)),
    free_terms(parse_free_terms(prototype, linkage)),
    free_ptterms(parse_busy_ptterms(prototype, free_terms)) {}

UIModule::UIModule(std::string const& _name,
                   Vector3f const& _pos) :
    UIObject(_name, _pos),
    prototype(nullptr),
    linkage(),
    free_ptterms() {}

/* printers */
void UIModule::print_to(std::ostream& os) const {
    os << "UIModule (" << name << ") [\n";
    os << "  module_name: " << module_name << "\n";
    os << "]";
}

}  /* elfin */