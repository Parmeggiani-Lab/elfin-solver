#include "arg_parser.h"

#include <sstream>

#include "json.h"
#include "debug_utils.h"

namespace elfin {

/* Private Methods */

ARG_PARSER_CALLBACK(help_and_exit) {
    raw("elfin: An Automatic Protein Designer\n");
    raw("Usage: ./elfin [OPTIONS]\n");
    raw("Note: Setting an option will override the same setting in the json file\n");
    raw("Help:\n");
    print_args();
    exit(1);
}

ARG_PARSER_CALLBACK(failure_callback) {
    printf("Argument parsing failed on string: \"%s\"\n", arg_in.c_str());
    help_and_exit();
    exit(1);
}

ARG_PARSER_CALLBACK(parse_config) {
    options_.config_file = arg_in;

    NICE_PANIC(!file_exists(options_.config_file.c_str()),
               string_format("Settings file does not exist: \"%s\"\n",
                             options_.config_file.c_str()));

    const JSON j = parse_json(options_.config_file);

    for (auto it = j.begin(); it != j.end(); ++it) {
        const std::string opt_name = "--" + it.key();
        auto ab = match_arg_bundle(opt_name.c_str());
        if (ab) {
            (this->*ab->callback)(json_to_str(j[ab->long_form]));
        } else {
            die("Unrecognized option: %s\n", opt_name.c_str());
        }
    }
}

ARG_PARSER_CALLBACK(set_input_file) {
    options_.input_file = arg_in;
}

void ArgParser::print_args() const {
    for (const auto & ab : argb_) {
        set_leading_spaces(8);
        raw("%s, %s\n", ab.short_form.c_str(), ab.long_form.c_str());
        set_leading_spaces(12);
        raw("%s\n", ab.description.c_str());
    }
    reset_leading_spaces();
}

const ArgBundle * ArgParser::match_arg_bundle(const char *arg_in) {
    // anything shorter than 2 chars cannot match
    if (arg_in[0] == '-')
        arg_in++; // to skip "-"

    for (const auto & ab : argb_) {
        if (!ab.short_form.compare(arg_in) or
                (arg_in[0] == '-' and !ab.long_form.compare((char *) (arg_in + 1)))
           ) {
            return &ab;
        }
    }

    return nullptr;
}

void ArgParser::parse_options(const int argc, char const *argv[]) {
    for (size_t i = 1; i < argc; i++) {
        // iterate through argument bundle to match argument
        auto arg = argv[i];
        auto ab = match_arg_bundle(arg);
        if (ab) {
            if (ab->exp_val) {
                if (i + 1 > argc - 1) {
                    err("Argument %s expects to be followed by a value\n", arg);
                    failure_callback(arg);
                }
                else {
                    (this->*ab->callback)(argv[++i]); // passes next arg string
                }
            }
            else {
                (this->*ab->callback)(arg); // passes current arg name
            }
        }
        else {
            err("Unknown argument: %s\n", arg);
            failure_callback(arg);
        }
    }
}

void ArgParser::check_options() const {
    // Files
    NICE_PANIC(options_.xdb == "",
               "No xdb file given. Check your settings.json\n");

    NICE_PANIC(!file_exists(options_.xdb.c_str()),
               "xdb file could not be found\n");

    NICE_PANIC(options_.input_file == "",
               "No input spec file given.\n");

    NICE_PANIC(!file_exists(options_.input_file.c_str()),
               "Input file could not be found\n");

    NICE_PANIC(options_.config_file == "",
               "No settings file file given.\n");

    NICE_PANIC(!file_exists(options_.config_file.c_str()),
               string_format("Settings file \"%s\" could not be found\n",
                             options_.config_file.c_str()));

    NICE_PANIC(options_.output_dir == "",
               "No output directory given.\n");

    if (!file_exists(options_.output_dir.c_str())) {
        wrn("Output directory does not exist; creating...\n");
        mkdir_ifn_exists(options_.output_dir.c_str());
    }

    // Settings

    NICE_PANIC(options_.ga_pop_size < 0, "Population size cannot be < 0\n");

    NICE_PANIC(options_.ga_iters < 0, "Number of iterations cannot be < 0\n");

    NICE_PANIC(options_.len_dev_alw < 0,
               "Gene length deviation must be an integer > 0\n");

    // GA params
    NICE_PANIC(options_.ga_survive_rate < 0.0 ||
               options_.ga_survive_rate > 1.0,
               "GA survive rate must be between 0 and 1 inclusive\n");
    NICE_PANIC(options_.ga_cross_rate < 0.0 ||
               options_.ga_cross_rate > 1.0,
               "GA cross rate must be between 0 and 1 inclusive\n");
    NICE_PANIC(options_.ga_point_mutate_rate < 0.0 ||
               options_.ga_point_mutate_rate > 1.0,
               "GA point mutate rate must be between 0 and 1 inclusive\n");
    NICE_PANIC(options_.ga_limb_mutate_rate < 0.0 ||
               options_.ga_limb_mutate_rate > 1.0,
               "GA limb mutate rate must be between 0 and 1 inclusive\n");

    NICE_PANIC(options_.avg_pair_dist < 0, "Average CoM distance must be > 0\n");

    NICE_PANIC(options_.keep_n < 0 ||
               options_.keep_n > options_.ga_pop_size,
               "Number of best solutions to output must be > 0 and < ga_pop_size\n");
}

void ArgParser::correct_rates() {
    bool rateCorrected = false;
    float sumRates = options_.ga_cross_rate +
                     options_.ga_point_mutate_rate +
                     options_.ga_limb_mutate_rate;
    if ((rateCorrected = (sumRates > 1.0))) {
        options_.ga_cross_rate         /= sumRates;
        options_.ga_point_mutate_rate   /= sumRates;
        options_.ga_limb_mutate_rate    /= sumRates;
        sumRates = options_.ga_cross_rate +
                   options_.ga_point_mutate_rate +
                   options_.ga_limb_mutate_rate;
    }

    if (rateCorrected) {
        wrn("Sum of GA cross + point mutate + limb mutate rates must be <= 1\n");
        wrn("Rates corrected to: %.2f, %.2f, %.2f\n",
            options_.ga_cross_rate,
            options_.ga_point_mutate_rate,
            options_.ga_limb_mutate_rate);
    }
}

/* Public Methods */

ArgParser::ArgParser(const int argc, char const *argv[]) {
    parse_options(argc, argv);
    check_options();
}

ArgParser::~ArgParser() {
}

Options ArgParser::get_options() const {
    return options_;
}

} // namespace elfin