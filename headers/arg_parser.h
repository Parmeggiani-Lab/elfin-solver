#ifndef ARG_PARSER_H_
#define ARG_PARSER_H_

#include <string>
#include <vector>
#include <memory>
#include <unordered_set>

#include "jutil.h"
#include "options.h"
#include "string_utils.h"

namespace elfin {

/* types */
class ArgParser;
typedef bool (ArgParser::*ArgBundleCallback)(std::string const&);

struct ArgBundle : public Printable {
    /* data */
    std::string const short_form;
    std::string const long_form;
    std::string const description;
    bool const exp_val;  // Will argument be followed by a value?
    ArgBundleCallback const callback;
    /* ctors */
    ArgBundle(std::string const&,
              std::string const&,
              std::string const&,
              bool const,
              ArgBundleCallback const&);
    /* printers */
    virtual void print_to(std::ostream& os) const;
};

/* Global Data */
extern std::unordered_set<std::string> const RADIUS_TYPES;

class ArgParser {
private:
    /* data */
    Options options_;
    char const* const config_file_long_from = "config_file";

    // Matching ArgBundle is O(n). Would be nice to do a map instead.
    std::vector<ArgBundle> const argb_ = {
        {   "h",
            "help",
            "Print this help text and exit.",
            false,
            &ArgParser::help_and_exit
        },
        {   "s",
            "spec_file",
            "Set input spec file - required argument.",
            true,
            &ArgParser::set_spec_file
        },
        {   "x",
            "xdb_file",
            string_format("Set xdb database file path (default=%s).",
            options_.xdb_file.c_str()),
            true,
            &ArgParser::set_xdb
        },
        {   "c",
            config_file_long_from,
            string_format("Set config file path (default=%s).",
            options_.config_file.c_str()),
            true,
            &ArgParser::parse_config
        },
        {   "o",
            "output_dir",
            string_format("Set output directory (default=%s).",
            options_.output_dir.c_str()),
            true,
            &ArgParser::set_output_dir
        },
        {   "os",
            "output_suffix",
            string_format("Set output file suffix (default=%s).",
            options_.output_suffix.c_str()),
            true,
            &ArgParser::set_output_suffix
        },
        {   "l",
            "len_dev",
            string_format("Set length deviation allowance (default=%zu).",
            options_.len_dev),
            true,
            &ArgParser::set_len_dev
        },
        {   "a",
            "avg_pair_dist",
            string_format(
                "Set average distance between CoMs "
                "(default=%.4f).",
                options_.avg_pair_dist),
            true,
            &ArgParser::set_avg_pair_dist
        },
        {   "cp",
            "collision_penalty",
            string_format("Set factor applied score when collision is detected (default=%.3f). "
            "Value of 0 means no penalty.",
            options_.collision_penalty),
            true,
            &ArgParser::set_collision_penalty
        },
        {   "r",
            "radius",
            string_format("Set radius type (default=%s).\n"
            "    Valid values are: %s.",
            options_.radius_type.c_str(),
            radius_types_setting_string().c_str()),
            true,
            &ArgParser::set_radius_type
        },
        {   "rf",
            "radius factor",
            string_format("Set radius factor for more or less sensitive collision detection (default=%.3f). "
            "Value of 0 is synonymous to no collision detection.",
            options_.radius_factor),
            true,
            &ArgParser::set_radius_factor
        },
        {   "S",
            "seed",
            string_format("Set RNG seed (default=0x%x). "
            "Value of 0 uses current time as seed.",
            options_.seed),
            true,
            &ArgParser::set_seed
        },
        {   "p",
            "ga_pop_size",
            string_format("Set GA population size (default=%zu).",
            options_.ga_pop_size),
            true,
            &ArgParser::set_ga_pop_size
        },
        {   "I",
            "ga_max_iters",
            string_format("Set max iterations for GA (default=%zu). "
            "\n    Values <= 0 means no limit.",
            options_.ga_max_iters),
            true,
            &ArgParser::set_ga_max_iters
        },
        {   "sr",
            "ga_survive_rate",
            string_format("Set GA survival rate (default=%.4f).",
            options_.ga_survive_rate),
            true,
            &ArgParser::set_ga_survive_rate
        },
        {   "sc",
            "ga_stop_score",
            string_format("Set GA exit score threshold (default=%.1f).",
            options_.ga_stop_score),
            true,
            &ArgParser::set_ga_stop_score
        },
        {   "rt",
            "ga_restart_trigger",
            string_format("Set number of stagnant generations "
            "before GA restarts (default=%zu)."
            "\n    Values <= 0 means no restarting.",
            options_.ga_restart_trigger),
            true, &ArgParser::set_ga_restart_trigger
        },
        {   "mr",
            "ga_max_restarts",
            string_format(
                "Set number of restarts before GA exits (default=%zu)."
                "\n    Values <= 0 means no exit by reason of too many restarts.",
                options_.ga_max_restarts),
            true, &ArgParser::set_ga_max_restarts
        },
        {   "v",
            "verbosity",
            string_format("Set log verbosity (default=%d). "
            "Valid values are (%d-%d).",
            JUtil.get_log_lvl(),
            LOGLVL_MIN + 1,
            LOGLVL_MAX - 1),
            true,
            &ArgParser::set_verbosity
        },
        // {   "d",
        //     "device",
        //     string_format("Run on accelerator device ID (default=%d).",
        //     options_.device),
        //     true,
        //     &ArgParser::set_device
        // },
        {   "w",
            "n_workers",
            string_format("Set number of worker threads  (default=%zu). "
            "\n    Value 0 uses the OMP_NUM_THREADS environment variable.",
            options_.n_workers),
            true,
            &ArgParser::set_n_workers
        },
        {   "k",
            "keep_n",
            string_format("Set number of best solutions to "
            "keep for output (default=%zu).",
            options_.keep_n),
            true,
            &ArgParser::set_keep_n
        },
        {   "dry",
            "dry_run",
            "Dry run mode - exit after initializing first population.",
            false,
            &ArgParser::set_dry_run
        },
        {   "t",
            "test",
            "Test mode - runs unit tests and integration tests.",
            false,
            &ArgParser::set_run_tests
        }
    };

    /* accessors */
    ArgBundle const* match_arg_bundle(char const* arg_in) const;
    void check_options() const;
    std::string radius_types_setting_string() const;

    /* modifiers */
    void parse_options(int const argc, char const* const argv[]);

// This macro must match signature of ArgBundleCallback.
#define ARG_CALLBACK_DECL(FUNC) \
    bool FUNC(std::string const& arg_in)

    ARG_CALLBACK_DECL(set_spec_file);
    ARG_CALLBACK_DECL(set_xdb);
    ARG_CALLBACK_DECL(parse_config);
    ARG_CALLBACK_DECL(set_output_dir);
    ARG_CALLBACK_DECL(set_output_suffix);
    ARG_CALLBACK_DECL(set_len_dev);
    ARG_CALLBACK_DECL(set_avg_pair_dist);
    ARG_CALLBACK_DECL(set_seed);
    ARG_CALLBACK_DECL(set_ga_pop_size);
    ARG_CALLBACK_DECL(set_ga_max_iters);
    ARG_CALLBACK_DECL(set_ga_survive_rate);
    ARG_CALLBACK_DECL(set_ga_stop_score);
    ARG_CALLBACK_DECL(set_ga_restart_trigger);
    ARG_CALLBACK_DECL(set_ga_max_restarts);
    ARG_CALLBACK_DECL(set_verbosity);
    ARG_CALLBACK_DECL(set_run_tests);
    ARG_CALLBACK_DECL(set_device);
    ARG_CALLBACK_DECL(set_n_workers);
    ARG_CALLBACK_DECL(set_keep_n);
    ARG_CALLBACK_DECL(set_dry_run);
    ARG_CALLBACK_DECL(set_radius_type);
    ARG_CALLBACK_DECL(set_radius_factor);
    ARG_CALLBACK_DECL(set_collision_penalty);

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