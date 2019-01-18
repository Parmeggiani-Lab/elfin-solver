#include "ui_module.h"

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

/* ctors */
UIModule::UIModule(std::string const& name,
                   JSON const& json) :
    UIObject(name, json),
    module_name(parse_module_name(json)),
    linkage(parse_linkage(json)) {}

UIModule::UIModule(std::string const& _name,
                   Vector3f const& _pos) :
    UIObject(_name, _pos) {}

/* printers */
void UIModule::print_to(std::ostream& os) const {
    os << "UIModule (" << name << ") [\n";
    os << "  module_name: " << module_name << "\n";
    os << "]";
}

}  /* elfin */