#include "arg_parser.h"

#include <regex>
#include <sstream>

#include "spec_parser.h"

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
    options_.configFile = arg_in;

    panic_if(!file_exists(options_.configFile.c_str()),
             "Settings file does not exist: \"%s\"\n",
             options_.configFile.c_str());

    const JSON j = parse_json(options_.configFile);

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
    options_.inputFile = arg_in;
    // Assume the input is a JSON file
    if (!(spec_ = SpecParser().parse(options_.inputFile))) {
        die("Failed to parse input spec.\n");
    }
}

void ArgParser::print_args() const {
    for (const auto & ab : argb_) {
        set_leading_spaces(8);
        raw("%s, %s\n", ab.short_form, ab.long_form);
        set_leading_spaces(12);
        raw("%s\n", ab.description);
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
    for (int i = 1; i < argc; i++) {
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
    // Do basic checks for each option

    // Files
    panic_if(options_.xdb == "",
             "No xDB file given. Check your settings.json\n");

    panic_if(!file_exists(options_.xdb.c_str()),
             "xDB file could not be found\n");

    panic_if(options_.inputFile == "",
             "No input spec file given. Check help using -h\n");

    panic_if(!file_exists(options_.inputFile.c_str()),
             "Input file could not be found\n");

    panic_if(options_.configFile == "",
             "No settings file file given. Check help using -h\n");

    panic_if(!file_exists(options_.configFile.c_str()),
             "Settings file \"%s\" could not be found\n",
             options_.configFile.c_str());

    panic_if(options_.outputDir == "",
             "No output directory given. Check help using -h\n");

    if (!file_exists(options_.outputDir.c_str())) {
        wrn("Output directory does not exist; creating...\n");
        mkdir_ifn_exists(options_.outputDir.c_str());
    }

    // Settings

    panic_if(options_.gaPopSize < 0, "Population size cannot be < 0\n");

    panic_if(options_.gaIters < 0, "Number of iterations cannot be < 0\n");

    panic_if(options_.lenDevAlw < 0,
             "Gene length deviation must be an integer > 0\n");

    // GA params
    panic_if(options_.gaSurviveRate < 0.0 ||
             options_.gaSurviveRate > 1.0,
             "GA survive rate must be between 0 and 1 inclusive\n");
    panic_if(options_.gaCrossRate < 0.0 ||
             options_.gaCrossRate > 1.0,
             "GA cross rate must be between 0 and 1 inclusive\n");
    panic_if(options_.gaPointMutateRate < 0.0 ||
             options_.gaPointMutateRate > 1.0,
             "GA point mutate rate must be between 0 and 1 inclusive\n");
    panic_if(options_.gaLimbMutateRate < 0.0 ||
             options_.gaLimbMutateRate > 1.0,
             "GA limb mutate rate must be between 0 and 1 inclusive\n");

    panic_if(options_.avgPairDist < 0, "Average CoM distance must be > 0\n");

    panic_if(options_.nBestSols < 0 ||
             options_.nBestSols > options_.gaPopSize,
             "Number of best solutions to output must be > 0 and < gaPopSize\n");
}

void ArgParser::correct_rates() {
    bool rateCorrected = false;
    float sumRates = options_.gaCrossRate +
                     options_.gaPointMutateRate +
                     options_.gaLimbMutateRate;
    if ((rateCorrected = (sumRates > 1.0))) {
        options_.gaCrossRate         /= sumRates;
        options_.gaPointMutateRate   /= sumRates;
        options_.gaLimbMutateRate    /= sumRates;
        sumRates = options_.gaCrossRate +
                   options_.gaPointMutateRate +
                   options_.gaLimbMutateRate;
    }

    if (rateCorrected) {
        wrn("Sum of GA cross + point mutate + limb mutate rates must be <= 1\n");
        wrn("Rates corrected to: %.2f, %.2f, %.2f\n",
            options_.gaCrossRate,
            options_.gaPointMutateRate,
            options_.gaLimbMutateRate);
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

std::shared_ptr<Spec> ArgParser::get_spec() const {
    return spec_;
}

} // namespace elfin