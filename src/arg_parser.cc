#include "arg_parser.h"

#include <sstream>

#include "json.h"
#include "debug_utils.h"
#include "exceptions.h"
#include "scoring.h"

namespace elfin {

/* Global Var Definition */
std::unordered_set<std::string> const RADIUS_TYPES = {
    "max_heavy_dist",
    "average_all",
    "max_ca_dist"
};

/* free functions */
void arg_parse_failure(std::string const& arg_in,
                       ArgBundle const* argb) {
    JUtil.error("Argument parsing failed on string: \"%s\"\n", arg_in.c_str());

    if (argb) {
        JUtil.error("Specific argument help:\n%s", argb->to_string().c_str());
    } else {
        JUtil.error("No related argument found.\n");
    }

    JUtil.error("Use -h flag for full help.\n");
    throw std::invalid_argument("argument parse failure");
}

/* ArgBundle */
ArgBundle::ArgBundle(std::string const& _short_form,
                     std::string const& _long_form,
                     std::string const& _desc,
                     bool const _exp_val,
                     ArgBundleCallback const& _callback) :
    short_form(_short_form),
    long_form(_long_form),
    description(_desc),
    exp_val(_exp_val),
    callback(_callback) {}

void ArgBundle::print_to(std::ostream& os) const {
    os << "  -" << short_form;
    os << ", --" << long_form << '\n';
    os << "    " << description << "\n";
}

/* ArgParser */
/* accessors */
ArgBundle const* ArgParser::match_arg_bundle(char const* arg_in) const {
    // Anything shorter than 2 chars cannot match
    if (arg_in[0] == '-') {
        arg_in++;  // Skip "-"
    }

    for (auto const& ab : argb_) {
        if (!ab.short_form.compare(arg_in) or
                (arg_in[0] == '-' and !ab.long_form.compare((char *) (arg_in + 1)))
           ) {
            return &ab;
        }
    }

    return nullptr;
}

void ArgParser::check_options() const {
    // Files.
    PANIC_IF(options_.xdb_file.empty(),
             BadArgument("No xdb path provided.\n"));

    PANIC_IF(not JUtil.file_exists(options_.xdb_file.c_str()),
             BadArgument("xdb file \""  +
                         options_.xdb_file +
                         "\" could not be found.\n"));

    PANIC_IF(options_.config_file != "" and
             not JUtil.file_exists(options_.config_file.c_str()),
             BadArgument("Settings file \"" +
                         options_.config_file +
                         "\" could not be found\n"));

    PANIC_IF(options_.output_dir.empty(),
             BadArgument("No output directory given."));

    if (not JUtil.file_exists(options_.output_dir.c_str())) {
        JUtil.warn("Output directory does not exist; creating...\n");
        JUtil.mkdir_ifn_exists(options_.output_dir.c_str());
    }

    // GA params.
    PANIC_IF(options_.ga_survive_rate <= 0.0 or
             options_.ga_survive_rate >= 1.0,
             BadArgument("GA survive rate must be between 0 and 1 exclusive.\n"));

    PANIC_IF(options_.avg_pair_dist < 0,
             BadArgument("Average CoM distance must be > 0.\n"));
}

std::string ArgParser::radius_types_setting_string() const {
    std::ostringstream oss;
    oss << "{ ";
    size_t count = 0;
    for (auto& rt : RADIUS_TYPES) {
        oss << rt;
        if (count++ < RADIUS_TYPES.size() - 1) {
            oss << ", ";
        }
    }
    oss << " }";
    return oss.str();
}

/* modifiers */
void ArgParser::parse_options(int const argc, char const* const argv[]) {
    // Skip binary name (index 0).
    for (size_t i = 1; i < argc; ++i) {
        // Iterate through argument bundle to match argument.
        auto arg = argv[i];
        auto ab = match_arg_bundle(arg);
        bool failed = false;
        if (ab) {
            if (ab->exp_val) {
                if (i + 1 > argc - 1) {
                    failed |= true;
                    JUtil.error("Argument %s requires a value\n", arg);
                }
                else {
                    // Parse value.
                    failed |= not (this->*ab->callback)(argv[++i]);
                }
            }
            else {
                // Parse flag.
                failed |= not (this->*ab->callback)(arg);
            }
        }
        else {
            JUtil.error("Unknown argument: %s\n", arg);
            failed |= true;
        }

        if (failed) {
            JUtil.error("Failed to parse options:\n");
            for (size_t j = 0; j < argc; ++j) {
                JUtil.error("argv[%zu]=%s\n", j, argv[j]);
            }
            arg_parse_failure(arg, ab);
        }
    }
}

// This macro must match signature of ArgBundleCallback.
#define ARG_PARSER_CALLBACK_DEF(FUNC_NAME) \
    bool ArgParser::FUNC_NAME(std::string const& arg_in)

ARG_PARSER_CALLBACK_DEF(set_spec_file) {
    options_.spec_file = arg_in;
    return true;
}

ARG_PARSER_CALLBACK_DEF(set_xdb) {
    options_.xdb_file = arg_in;
    return true;
}

ARG_PARSER_CALLBACK_DEF(parse_config) {
    options_.config_file = arg_in;

    PANIC_IF(not JUtil.file_exists(options_.config_file.c_str()),
             BadArgument("Settings file \"" +
                         options_.config_file +
                         "\" does not exist.\n"));

    JSON const json = parse_json(options_.config_file);

    for (auto& [opt_key, opt_json] : json.items()) {
        if (opt_key == config_file_long_from) {
            JUtil.warn("Ignoring %s in config JSON.\n", config_file_long_from);
            continue;
        }

        std::string const opt_name = "--" + opt_key;
        auto ab = match_arg_bundle(opt_name.c_str());
        if (ab) {
            (this->*ab->callback)(json_to_clean_str(json[ab->long_form]));
        } else {
            JUtil.error("Unrecognized option: %s\n", opt_name.c_str());
            arg_parse_failure(opt_name.c_str(), ab);
            return true;
        }
    }

    return true;
}

ARG_PARSER_CALLBACK_DEF(set_output_dir) {
    options_.output_dir = arg_in;
    return true;
}

ARG_PARSER_CALLBACK_DEF(set_len_dev) {
    long const l = JUtil.parse_long(arg_in.c_str());
    options_.len_dev = l < 0 ? 0 : l;
    return true;
}

ARG_PARSER_CALLBACK_DEF(set_avg_pair_dist) {
    options_.avg_pair_dist = JUtil.parse_float(arg_in.c_str());
    return true;
}

ARG_PARSER_CALLBACK_DEF(set_seed) {
    options_.seed = JUtil.parse_long(arg_in.c_str());
    return true;
}

ARG_PARSER_CALLBACK_DEF(set_ga_pop_size) {
    long const l = JUtil.parse_long(arg_in.c_str());
    options_.ga_pop_size = l < 0 ? 0 : l;
    return true;
}

ARG_PARSER_CALLBACK_DEF(set_ga_max_iters) {
    long const l = JUtil.parse_long(arg_in.c_str());
    options_.ga_max_iters = l < 0 ? 0 : l;
    return true;
}

ARG_PARSER_CALLBACK_DEF(set_ga_survive_rate) {
    options_.ga_survive_rate = JUtil.parse_float(arg_in.c_str());
    return true;
}

ARG_PARSER_CALLBACK_DEF(set_ga_stop_score) {
    options_.ga_stop_score =
        std::max((double) JUtil.parse_float(arg_in.c_str()),
                 scoring::SCORE_FLOOR);
    return true;
}

ARG_PARSER_CALLBACK_DEF(set_ga_restart_trigger) {
    long const l = JUtil.parse_long(arg_in.c_str());
    options_.ga_restart_trigger = l < 0 ? 0 : l;
    return true;
}

ARG_PARSER_CALLBACK_DEF(set_ga_max_restarts) {
    long const l = JUtil.parse_long(arg_in.c_str());
    options_.ga_max_restarts = l < 0 ? 0 : l;
    return true;
}

ARG_PARSER_CALLBACK_DEF(set_verbosity) {
    // Call jutil function to set global log level.
    long const l = JUtil.parse_long(arg_in.c_str());
    JUtil.set_log_lvl(static_cast<JUtilLogLvl>(l < 0 ? 0 : l));
    return true;
}

ARG_PARSER_CALLBACK_DEF(set_run_tests) {
    options_.run_tests = true;
    options_.spec_file = "examples/quarter_snake_free.json";
    return true;
}

ARG_PARSER_CALLBACK_DEF(set_device) {
    options_.device = JUtil.parse_long(arg_in.c_str());
    return true;
}

ARG_PARSER_CALLBACK_DEF(set_n_workers) {
    long const l = JUtil.parse_long(arg_in.c_str());
    options_.n_workers = l < 0 ? 0 : l;
    return true;
}

ARG_PARSER_CALLBACK_DEF(set_keep_n) {
    long const l = JUtil.parse_long(arg_in.c_str());
    options_.keep_n = l < 0 ? 0 : l;
    return true;
}

ARG_PARSER_CALLBACK_DEF(set_dry_run) {
    options_.dry_run = true;
    return true;
}

ARG_PARSER_CALLBACK_DEF(set_radius_type) {
    bool const radiu_type_is_valid =
        RADIUS_TYPES.find(arg_in) != end(RADIUS_TYPES);

    if (radiu_type_is_valid) {
        options_.radius_type = arg_in;
    }
    else {
        JUtil.error("Invalid radius type: \"%s\"\n", arg_in.c_str());
    }

    return radiu_type_is_valid;
}

/* printers */
ARG_PARSER_CALLBACK_DEF(help_and_exit) {
    std::stringstream ss;

    ss << "\nelfin-solver: an protein structure design solver\n";
    ss << "Report issues at: https://github.com/joy13975/elfin-solver/issues\n\n";
    ss << "Usage: ./elfin [OPTIONS]\n";
    ss << "Note: settings are parsed and overridden in the order they're written\n";
    ss << "OPTIONS:\n";

    for (auto const& ab : argb_) {
        ss << ab.to_string();
    }

    std::cout << ss.rdbuf();
    throw ExitException(0, "Help nessage printed.");

    return false;  // Suppress warning.
}

#undef ARG_PARSER_CALLBACK_DEF

/* public */
/* ctors */
ArgParser::ArgParser(int const argc, char const* const argv[]) {
    parse_options(argc, argv);
    check_options();
}

} // namespace elfin