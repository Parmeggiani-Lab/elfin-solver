/* Copyright 2018 Joy Yeh <joyyeh@gmail.com> */

#include "elfin.h"

#include <string>
#include <sstream>
#include <csignal>
#include <iostream>
#include <fstream>

#include "options.h"
#include "spec.h"
#include "arg_parser.h"
#include "db_parser.h"
#include "math_utils.h"
#include "kabsch.h"
#include "jutil.h"
#include "random_utils.h"
#include "parallel_utils.h"

#ifndef _NO_OMP
#include <omp.h>
#endif

namespace elfin {

/* link global references to data members */
Options options_;
Spec spec_;
const Options & OPTIONS = options_;
const Spec & SPEC = spec_;

std::vector<ElfinRunner *> ElfinRunner::instances_;
bool interrupt_caught = false;

// https://stackoverflow.com/questions/8520560/get-a-file-name-from-a-path
std::string get_filename(const std::string & path) {
    std::string filename = path;
    // Remove directory if present.
    // Do this before extension removal incase directory has a period character.
    const size_t last_slash_idx = filename.find_last_of("\\/");
    if (std::string::npos != last_slash_idx) {
        filename.erase(0, last_slash_idx + 1);
    }

    // Remove extension if present.
    const size_t period_idx = filename.rfind('.');
    if (std::string::npos != period_idx) {
        filename.erase(period_idx);
    }
    return filename;
}

void ElfinRunner::interrupt_handler(const int signal) {
    if (interrupt_caught) {
        raw("\n\n");
        die("Caught interrupt signal (second)\n");
    }
    else {
        interrupt_caught = true;

        raw("\n\n");
        wrn("Caught interrupt signal (first)\n");

        // Save latest results
        for (auto er : instances_) {
            er->crash_dump();
            delete er;
        }

        exit(signal);
    }
}

void ElfinRunner::write_output(std::string alt_dir) const {
    if (alt_dir.length() > 0) {
        mkdir_ifn_exists(alt_dir.c_str());
        alt_dir += "/";
    }

    JSON data;
    for (auto & kv : SPEC.work_areas()) {
        const std::string & wa_name = kv.first;
        const WorkArea & wa = kv.second;
        JSON waj;

        try {
            auto & candidates = es_->best_sols().at(wa_name);
            for (size_t i = 0; i < candidates.size(); ++i)
            {
                JSON cand_json;
                auto c = candidates.at(i);

                if (c) {
                    auto node_names = c->get_node_names();
                    cand_json["nodes"] = node_names;
                    wrn("Output format not complete\n");
                    cand_json["score"] = c->get_score();
                }
                else {
                    err("Null candidate in work area \"%s\"\n", wa_name.c_str());
                }

                waj[i] = cand_json;
            }
            data[wa_name] = waj;
        }
        catch (std::out_of_range& e)
        {
            wrn("WorkArea \"%s\" has no solutions\n", wa_name.c_str());
        }
    }

    // format json output path
    std::ostringstream json_output_path_ss;
    json_output_path_ss << alt_dir
                        << options_.output_dir << "/"
                        << get_filename(options_.input_file)
                        << ".json";

    std::string json_out_path_str = json_output_path_ss.str();
    const char * json_output_path = json_out_path_str.c_str();

    // write json
    std::string dump = data.dump();
    write_binary(json_output_path,
                 dump.c_str(),
                 dump.size());
}

void ElfinRunner::crash_dump() const {
    if (es_started_) {
        wrn("Dumping latest results...\n");
        write_output("crash_dump");
        delete es_;
    } else {
        wrn("GA did not get to start\n");
    }
}

int ElfinRunner::run_unit_tests() const {
    msg("Running unit tests...\n");
    int fail_count = 0;
    fail_count += _test_math_utils();
    fail_count += _test_kabsch();
    return fail_count;
}

int ElfinRunner::run_meta_tests() const {
    msg("Running meta tests...\n");
    int fail_count = 0;

    for (auto & itr : SPEC.work_areas()) {
        const WorkArea & wa = itr.second;
        V3fList moved_spec = wa.to_V3fList();

        Vector3f rot_arr[3] = {
            Vector3f(1.0f, 0.0f, 0.0f),
            Vector3f(0.0f, -0.5177697998f, 0.855519979f),
            Vector3f(0.0f, -0.855519979f, -0.5177697998f)
        };
        Mat3x3 rot_around_x = Mat3x3(rot_arr);

        // It seems that Kabsch cannot handle very large
        // translations, but in the scale we're working
        // at it should rarely, if ever, go beyond
        // one thousand Angstroms
        Vector3f tran(-39.0f, 999.3413f, -400.11f);

        for (Vector3f &p : moved_spec) {
            p = p.dot(rot_around_x);
            p += tran;
        }

        // Test scoring a transformed version of spec
        const float trx_score = kabsch_score(moved_spec, wa.to_V3fList());
        if (!float_approximates(trx_score, 0)) {
            fail_count++;
            wrn("Self score test failed: self score should be 0\n");
        }

        // Test randomiser
        const int N = 10;
        const int rand_trials = 50000000;
        const int expect_avg = rand_trials / N;
        const float rand_dev_tolerance = 0.05f * expect_avg;  // 5% deviation

        int rand_count[N] = {0};
        for (size_t i = 0; i < rand_trials; i++) {
            const size_t dice = get_dice(N);
            if (dice >= N) {
                fail_count++;
                err("Failed to produce correct dice: get_dice() "
                    "produced %d for [0-%d)",
                    dice, N);
                break;
            }
            rand_count[dice]++;
        }

        for (size_t i = 0; i < N; i++) {
            const float rand_dev = static_cast<float>(
                                       abs(rand_count[i] - expect_avg) / (expect_avg));
            if (rand_dev > rand_dev_tolerance) {
                fail_count++;
                err("Too much random deviation: %.3f%% (expecting %d)\n",
                    rand_dev, expect_avg);
            }
        }

        // Test parallel randomiser
#ifndef _NO_OMP
        std::vector<uint> para_rand_seeds = get_para_rand_seeds();
        const int n_threads = para_rand_seeds.size();
        const int para_rand_n = 8096;
        const int64_t dice_lim = 13377331;

        std::vector<uint> rands1(para_rand_n);
        #pragma omp parallel for
        for (size_t i = 0; i < para_rand_n; i++)
            rands1.at(i) = get_dice(dice_lim);

        get_para_rand_seeds() = para_rand_seeds;
        std::vector<uint> rands2(para_rand_n);
        #pragma omp parallel for
        for (size_t i = 0; i < para_rand_n; i++)
            rands2.at(i) = get_dice(dice_lim);

        for (size_t i = 0; i < para_rand_n; i++) {
            if (rands1.at(i) != rands2.at(i)) {
                fail_count++;
                err("Parallel randomiser failed: %d vs %d\n",
                    rands1.at(i), rands2.at(i));
            }
        }
#endif

        if (fail_count > 0)
            break;
    }

    return fail_count;
}

ElfinRunner::ElfinRunner(const int argc, const char ** argv) {
    instances_.push_back(this);

    std::signal(SIGINT, interrupt_handler);

    // Default set to warning and above
    set_log_level(LOG_WARN);

    ArgParser ap(argc, argv);
    options_ = ap.get_options();

    mkdir_ifn_exists(options_.output_dir.c_str());

    msg("Using master seed: %d\n", options_.rand_seed);

    DBParser::parse(parse_json(options_.xdb));

    set_thread_seeds(options_.rand_seed);

    spec_.parse_from_json(parse_json(options_.input_file));
}

void ElfinRunner::run() {
    if (options_.run_unit_tests) {
        int fail_count = 0;
        fail_count += run_unit_tests();
        fail_count += run_meta_tests();

        if (fail_count > 0) {
            die("Some unit tests failed\n");
        } else {
            msg("Passed!\n");
        }
    } else {
        es_ = new EvolutionSolver();
        es_started_ = true;
        es_->run();
        write_output();

        delete es_;
    }
}

}  // namespace elfin

int main(const int argc, const char ** argv) {
    elfin::ElfinRunner(argc, argv).run();
    return 0;
}
