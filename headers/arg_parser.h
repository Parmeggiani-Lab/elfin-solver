#ifndef ARG_PARSER_H_
#define ARG_PARSER_H_

#include <string>
#include <vector>
#include <memory>

#include <jutil/jutil.h>
#include "options.h"

namespace elfin {

/* types */
class ArgParser;
typedef void (ArgParser::*ArgBundleCallback)(std::string const&);

struct ArgBundle {
    std::string const short_form;
    std::string const long_form;
    std::string const description;
    bool const exp_val;  // Will argument be followed by a value?
    ArgBundleCallback const callback;
    std::string to_string() const;
};

class ArgParser {
private:
    /* data */
    Options options_;

    // Matching ArgBundle is O(n). Would be nice to do a map instead.
    std::vector<ArgBundle> argb_ = {
        {   "h",
            "help",
            "Print this help text and exit.",
            false,
            &ArgParser::help_and_exit
        },
        {   "c",
            "config_file",
            "Set config file path (default=\"\").",
            true,
            &ArgParser::parse_config
        },
        {   "s",
            "spec_file",
            "Set input specification file - compulsory argument.",
            true,
            &ArgParser::set_spec_file
        },
        {   "x",
            "xdb",
            "Set xdb database file path (default=./xdb.json).",
            true,
            &ArgParser::set_xdb
        },
        {   "o",
            "output_dir",
            "Set output directory (default=./output/).",
            true,
            &ArgParser::set_output_dir
        },
        {   "l",
            "len_dev",
            "Set length deviation allowance (default=3).",
            true,
            &ArgParser::set_len_dev
        },
        {   "a",
            "avg_pair_dist",
            ("Overwrite average distance between doubles of CoMs "
            "(default=~37.71)."),
            true,
            &ArgParser::set_avg_pair_dist
        },
        {   "S",
            "seed",
            ("Set RNG seed (default=0x1337cafe). "
            "Value of 0 uses time as seed."),
            true,
            &ArgParser::set_seed
        },
        {   "p",
            "ga_pop_size",
            "Set GA population size (default=8096).",
            true,
            &ArgParser::set_ga_pop_size
        },
        {   "I",
            "ga_iters",
            "Set GA iterations (default=1000).",
            true,
            &ArgParser::set_ga_iters
        },
        {   "sv",
            "ga_survive_rate",
            "Set GA survival rate (default=0.05).",
            true,
            &ArgParser::set_ga_survive_rate
        },
        {   "sc",
            "ga_stop_score",
            "Set GA exit score threshold (default=0.001).",
            true,
            &ArgParser::set_ga_stop_score
        },
        {   "st",
            "ga_stop_stagnancy",
            ("Set number of stagnant generations "
            "before GA exits (default=50). Value of -1 means "
            "no stopping by reason of stagnancy."),
            true, &ArgParser::set_ga_stop_stagnancy
        },
        {   "v",
            "verbosity",
            "Set log verbosity (default=1). Valid values are (0-2).",
            true,
            &ArgParser::set_verbosity
        },
        {   "t",
            "test",
            "Run unit tests.",
            false,
            &ArgParser::set_run_tests
        },
        {   "d",
            "device",
            "Run on accelerator device ID (default=0).",
            true,
            &ArgParser::set_device
        },
        {   "w",
            "n_workers",
            "Set number of worker threads (default=0, which uses the OMP_NUM_THREADS env var).",
            true,
            &ArgParser::set_n_workers
        },
        {   "k",
            "keep_n",
            "Set number of best solutions to keep for output (default=3).",
            true,
            &ArgParser::set_keep_n
        },
        {   "dry",
            "dry_run",
            "Use dry run mode - exit after initializing first population.",
            false,
            &ArgParser::set_dry_run
        },
        {   "R",
            "radius",
            ("Set radius type to one of {max_ca_dist (default), "
            "max_heavy_dist, average_all}."),
            true,
            &ArgParser::set_radius_type
        }
    };

    /* accessors */
    ArgBundle const* match_arg_bundle(char const* arg_in) const;
    void check_options() const;

    /* modifiers */
    void parse_options(int const argc, char const *argv[]);

#define ARG_CALLBACK_DECL(FUNC) \
    void FUNC(std::string const& arg_in)

    ARG_CALLBACK_DECL(parse_config);

    ARG_CALLBACK_DECL(set_spec_file);

    ARG_CALLBACK_DECL(set_xdb) {
        options_.xdb = arg_in;
    }

    ARG_CALLBACK_DECL(set_output_dir) {
        options_.output_dir = arg_in;
    }

    ARG_CALLBACK_DECL(set_len_dev) {
        options_.len_dev = JUtil.parse_long(arg_in.c_str());
    }

    ARG_CALLBACK_DECL(set_avg_pair_dist) {
        options_.avg_pair_dist = JUtil.parse_float(arg_in.c_str());
    }

    ARG_CALLBACK_DECL(set_seed) {
        options_.seed = JUtil.parse_long(arg_in.c_str());
    }

    ARG_CALLBACK_DECL(set_ga_pop_size) {
        options_.ga_pop_size = JUtil.parse_long(arg_in.c_str());
    }

    ARG_CALLBACK_DECL(set_ga_iters) {
        options_.ga_iters = JUtil.parse_long(arg_in.c_str());
    }

    ARG_CALLBACK_DECL(set_ga_survive_rate) {
        options_.ga_survive_rate = JUtil.parse_float(arg_in.c_str());
    }

    ARG_CALLBACK_DECL(set_ga_stop_score) {
        options_.ga_stop_score = JUtil.parse_float(arg_in.c_str());
    }

    ARG_CALLBACK_DECL(set_ga_stop_stagnancy) {
        options_.ga_stop_stagnancy = JUtil.parse_long(arg_in.c_str());
    }

    ARG_CALLBACK_DECL(set_verbosity) {
        // Call jutil function to set global log level.
        JUtil.set_log_level((LogLvl) JUtil.parse_long(arg_in.c_str()));
    }

    ARG_CALLBACK_DECL(set_run_tests) {
        options_.run_tests = true;
        options_.spec_file = "examples/quarter_snake_free.json";
    }

    ARG_CALLBACK_DECL(set_device) {
        options_.device = JUtil.parse_long(arg_in.c_str());
    }

    ARG_CALLBACK_DECL(set_n_workers) {
        options_.n_workers = JUtil.parse_long(arg_in.c_str());
    }

    ARG_CALLBACK_DECL(set_keep_n) {
        options_.keep_n = JUtil.parse_long(arg_in.c_str());
    }

    ARG_CALLBACK_DECL(set_dry_run) {
        options_.dry_run = true;
    }

    ARG_CALLBACK_DECL(set_radius_type) {
        options_.radius_type = arg_in;
    }

    /* printers */
    ARG_CALLBACK_DECL(help_and_exit);

#undef ARG_CALLBACK_DECL

public:
    /* ctors */
    ArgParser(int const argc, char const * const argv[]);
    Options get_options() const { return options_; }
};

}

#endif /* end of include guard: ARG_PARSER_H_ */