#ifndef ARG_PARSER_H_
#define ARG_PARSER_H_

#include <string>
#include <vector>
#include <memory>

#include <jutil/jutil.h>
#include "options.h"

#define ARG_PARSER_CALLBACK_PARAMETERS \
    const std::string && arg_in
#define ARG_PARSER_CALLBACK(name) \
    void ArgParser::name(ARG_PARSER_CALLBACK_PARAMETERS)
#define ARG_PARSER_CALLBACK_IN_HEADER(name) \
    void name(ARG_PARSER_CALLBACK_PARAMETERS="")

namespace elfin {

class ArgParser;

typedef void (ArgParser::*ArgBundleCallback)(ARG_PARSER_CALLBACK_PARAMETERS);

struct ArgBundle {
    std::string short_form;
    std::string long_form;
    std::string description;
    bool exp_val;  // Will argument be followed by a value?
    ArgBundleCallback callback;
};

class ArgParser {
private:
    /* data */
    Options options_;

    // Matching ArgBundle is O(n). Would be nice to do a map instead.
    std::vector<ArgBundle> argb_ = {
        {   "h",
            "help",
            "Print this help text and exit",
            false,
            &ArgParser::help_and_exit
        },
        {   "c",
            "set_config_file",
            "Set config file path (default=None)",
            true,
            &ArgParser::parse_config
        },
        {   "i",
            "input_file",
            "Set input file path (default=None) - compulsory paramter",
            true,
            &ArgParser::set_input_file
        },
        {   "x",
            "xdb",
            "Set xdb database file path (default=./xdb.json)",
            true,
            &ArgParser::set_xdb
        },
        {   "o",
            "output_dir",
            "Set output directory (default=./output/)",
            true,
            &ArgParser::set_output_dir
        },
        {   "l",
            "len_dev_alw",
            "Set length deviation allowance (default=3)",
            true,
            &ArgParser::set_len_dev_alw
        },
        {   "a",
            "avg_pair_dist",
            ("Overwrite average distance between doubles of CoMs "
            "(default=~37.71)"),
            true,
            &ArgParser::set_avg_pair_dist
        },
        {   "s",
            "rand_seed",
            ("Set RNG seed (default=0x1337cafe). Value of 0 uses time as "
            "seed)"),
            true,
            &ArgParser::set_rand_seed
        },
        {   "gps",
            "ga_pop_size",
            "Set GA population size (default=10000)",
            true,
            &ArgParser::set_ga_pop_size
        },
        {   "git",
            "ga_iters",
            "Set GA iterations (default=1000)",
            true,
            &ArgParser::set_ga_iters
        },
        {   "gsr",
            "ga_survive_rate",
            "Set GA survival rate (default=0.05)",
            true,
            &ArgParser::set_ga_survive_rate
        },
        {   "stt",
            "ga_stop_score",
            "Set GA exit score threshold (default=0.001)",
            true,
            &ArgParser::set_ga_stop_score
        },
        {   "msg",
            "ga_stop_stagnancy",
            ("Set number of stagnant generations "
            "before GA exits (default=50). Value of -1 means no stop by "
            "stagnancy."),
            true, &ArgParser::set_ga_stop_stagnancy
        },
        {   "lg",
            "log_level",
            "Set log level (default=LOG_WARN=2). Value ranges is (0-7)",
            true,
            &ArgParser::set_log_level
        },
        {   "t",
            "test",
            "Run unit tests",
            false,
            &ArgParser::set_run_unit_tests
        },
        {   "dv",
            "device",
            "Run on accelerator device ID (default=0)",
            true,
            &ArgParser::set_device
        },
        {   "w",
            "n_workers",
            "Set number of threads (default=0, which uses the OMP_NUM_THREADS env var)",
            true,
            &ArgParser::set_n_workers
        },
        {   "kn",
            "keep_n",
            "Set number of best solutions to keep for output (default=3)",
            true,
            &ArgParser::set_keep_n
        },
        {   "dry",
            "dry_run",
            "Enable dry run - exit just after initializing first population",
            false,
            &ArgParser::set_dry_run
        },
        {   "r",
            "radius",
            ("Set radius type from one of {max_ca_dist (default), "
            "max_heavy_dist, average_all}"),
            true,
            &ArgParser::set_radius_type
        }
    };

    /* accessors */
    ArgBundle const* match_arg_bundle(char const* arg_in) const;
    void check_options() const;

    /* modifiers */
    void parse_options(int const argc, char const *argv[]);

    ARG_PARSER_CALLBACK_IN_HEADER(help_and_exit);

    ARG_PARSER_CALLBACK_IN_HEADER(failure_callback);

    ARG_PARSER_CALLBACK_IN_HEADER(parse_config);

    ARG_PARSER_CALLBACK_IN_HEADER(set_input_file);

    ARG_PARSER_CALLBACK_IN_HEADER(set_xdb) {
        options_.xdb = arg_in;
    }

    ARG_PARSER_CALLBACK_IN_HEADER(set_output_dir) {
        options_.output_dir = arg_in;
    }

    ARG_PARSER_CALLBACK_IN_HEADER(set_len_dev_alw) {
        options_.len_dev_alw = parse_long(arg_in.c_str());
    }

    ARG_PARSER_CALLBACK_IN_HEADER(set_avg_pair_dist) {
        options_.avg_pair_dist = parse_float(arg_in.c_str());
    }

    ARG_PARSER_CALLBACK_IN_HEADER(set_rand_seed) {
        options_.rand_seed = parse_long(arg_in.c_str());
    }


    ARG_PARSER_CALLBACK_IN_HEADER(set_ga_pop_size) {
        options_.ga_pop_size = parse_long(arg_in.c_str());
    }

    ARG_PARSER_CALLBACK_IN_HEADER(set_ga_iters) {
        options_.ga_iters = parse_long(arg_in.c_str());
    }

    ARG_PARSER_CALLBACK_IN_HEADER(set_ga_survive_rate) {
        options_.ga_survive_rate = parse_float(arg_in.c_str());
    }

    ARG_PARSER_CALLBACK_IN_HEADER(set_ga_stop_score) {
        options_.ga_stop_score = parse_float(arg_in.c_str());
    }

    ARG_PARSER_CALLBACK_IN_HEADER(set_ga_stop_stagnancy) {
        options_.ga_stop_stagnancy = parse_long(arg_in.c_str());
    }

    ARG_PARSER_CALLBACK_IN_HEADER(set_log_level) {
        ::set_log_level((Log_Level) parse_long(arg_in.c_str()));
    }

    ARG_PARSER_CALLBACK_IN_HEADER(set_run_unit_tests) {
        options_.run_unit_tests = true;
        options_.input_file = "examples/quarter_snake_free.json";
    }

    ARG_PARSER_CALLBACK_IN_HEADER(set_device) {
        options_.device = parse_long(arg_in.c_str());
    }

    ARG_PARSER_CALLBACK_IN_HEADER(set_n_workers) {
        options_.n_workers = parse_long(arg_in.c_str());
    }

    ARG_PARSER_CALLBACK_IN_HEADER(set_keep_n) {
        options_.keep_n = parse_long(arg_in.c_str());
    }

    ARG_PARSER_CALLBACK_IN_HEADER(set_dry_run) {
        options_.dry_run = true;
    }

    ARG_PARSER_CALLBACK_IN_HEADER(set_radius_type) {
        options_.radius_type = arg_in;
    }

    /* printers */
    void print_args() const;

public:
    /* ctors */
    ArgParser(int const argc, char const *argv[]);
    Options get_options() const { return options_; }
};

}

#endif /* end of include guard: ARG_PARSER_H_ */