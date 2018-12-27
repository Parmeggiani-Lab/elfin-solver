#include "arg_parser.h"

#include <sstream>

#include "json.h"
#include "debug_utils.h"

namespace elfin {

/* free functions */
void failure_callback(
    std::string const& arg_in,
    ArgBundle const* argb) {
    JUtil.error("Argument parsing failed on string: \"%s\"\n", arg_in.c_str());

    if (argb) {
        JUtil.error("Specific argument help:\n%s", argb->to_string().c_str());
    } else {
        JUtil.error("No related argument found.\n");
    }

    JUtil.error("Use -h flag for full help.\n");
    exit(1);
}

/* ArgBundle */
std::string ArgBundle::to_string() const {
    std::ostringstream ss;
    ss << "  -" << short_form;
    ss << ", --" << long_form << '\n';
    ss << "    " << description << "\n";
    return ss.str();
}

/* ArgParser */
/* accessors */
ArgBundle const* ArgParser::match_arg_bundle(char const* arg_in) const {
    // Anything shorter than 2 chars cannot match
    if (arg_in[0] == '-') {
        arg_in++; // Skip "-"
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
    JUtil.panic_if(
        options_.xdb == "",
        "No xdb path provided.\n");

    JUtil.panic_if(
        not JUtil.file_exists(options_.xdb.c_str()),
        "xdb file could not be found.\n");

    JUtil.panic_if(options_.spec_file == "",
                   "No input spec file given.\n");

    JUtil.panic_if(
        not JUtil.file_exists(options_.spec_file.c_str()),
        "Input file could not be found.\n");

    if (options_.config_file != "") {
        JUtil.panic_if(
            not JUtil.file_exists(options_.config_file.c_str()),
            "Settings file \"%s\" could not be found\n",
            options_.config_file.c_str());
    }

    JUtil.panic_if(
        options_.output_dir == "",
        "No output directory given.");

    if (not JUtil.file_exists(options_.output_dir.c_str())) {
        JUtil.warn("Output directory does not exist; creating...");
        JUtil.mkdir_ifn_exists(options_.output_dir.c_str());
    }

    // Settings.
    JUtil.panic_if(options_.ga_pop_size < 0,
                   "Population size cannot be < 0.\n");

    JUtil.panic_if(options_.ga_iters < 0,
                   "Number of iterations cannot be < 0.\n");

    JUtil.panic_if(options_.len_dev < 0,
                   "Candidate length deviation must be an integer > 0.\n");

    // GA params.
    JUtil.panic_if(options_.ga_survive_rate <= 0.0 or
                   options_.ga_survive_rate >= 1.0,
                   "GA survive rate must be between 0 and 1 exclusive.\n");

    JUtil.panic_if(options_.avg_pair_dist < 0,
                   "Average CoM distance must be > 0");

    JUtil.panic_if(options_.keep_n < 0 or
                   options_.keep_n > options_.ga_pop_size,
                   "Number of best solutions to output must be > 0 and < ga_pop_size.\n");

    JUtil.panic_if(options_.radius_type != "max_heavy_dist" and
                   options_.radius_type != "average_all" and
                   options_.radius_type != "max_ca_dist",
                   "In valid radius type: \"%s\"\n", options_.radius_type.c_str());
}

/* modifiers */
void ArgParser::parse_options(int const argc, char const* const argv[]) {
    for (size_t i = 1; i < argc; ++i) {
        // iterate through argument bundle to match argument
        auto arg = argv[i];
        auto ab = match_arg_bundle(arg);
        if (ab) {
            if (ab->exp_val) {
                if (i + 1 > argc - 1) {
                    JUtil.error("Argument %s expects to be followed by a value\n", arg);
                    failure_callback(arg, ab);
                }
                else {
                    (this->*ab->callback)(argv[++i]); // Passes next arg string.
                }
            }
            else {
                (this->*ab->callback)(arg); // Passes current arg name.
            }
        }
        else {
            JUtil.error("Unknown argument: %s\n", arg);
            failure_callback(arg, ab);
        }
    }
}


#define ARG_PARSER_CALLBACK_DEF(name) \
    void ArgParser::name(std::string const& arg_in)

ARG_PARSER_CALLBACK_DEF(parse_config) {
    options_.config_file = arg_in;

    JUtil.panic_if(
        not JUtil.file_exists(options_.config_file.c_str()),
        "Settings file does not exist: \"%s\"\n",
        options_.config_file.c_str());

    JSON const j = parse_json(options_.config_file);

    for (auto it = begin(j); it != end(j); ++it) {
        const std::string opt_name = "--" + it.key();
        auto ab = match_arg_bundle(opt_name.c_str());
        if (ab) {
            (this->*ab->callback)(json_to_str(j[ab->long_form]));
        } else {
            die("Unrecognized option: %s\n", opt_name.c_str());
        }
    }
}

ARG_PARSER_CALLBACK_DEF(set_spec_file) {
    options_.spec_file = arg_in;
}

/* printers */
ARG_PARSER_CALLBACK_DEF(help_and_exit) {
    JUtil.gated_log(LOGLVL_INFO,
                    "",
                    "\nelfin-solver: an protein structure design solver\n");
    JUtil.gated_log(LOGLVL_INFO,
                    "",
                    "Report issues at: https://github.com/joy13975/elfin-solver/issues\n\n");
    JUtil.gated_log(LOGLVL_INFO,
                    "",
                    "Usage: ./elfin [OPTIONS]\n");
    JUtil.gated_log(LOGLVL_INFO,
                    "",
                    "Note: settings are parsed and overridden in the order they're written\n");
    JUtil.gated_log(LOGLVL_INFO,
                    "",
                    "OPTIONS:\n");
    for (auto const& ab : argb_) {
        JUtil.gated_log(LOGLVL_INFO,
                        "",
                        "%s", ab.to_string().c_str());
    }
    exit(1);
}

#undef ARG_PARSER_CALLBACK_DEF

/* public */
/* ctors */
ArgParser::ArgParser(int const argc, char const *argv[]) {
    parse_options(argc, argv);
    check_options();
}

} // namespace elfin