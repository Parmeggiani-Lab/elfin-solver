/* Copyright 2018 Joy Yeh <joyyeh@gmail.com> */

#include "../src/elfin.h"

#include <string>
#include <sstream>
#include <csignal>
#include <iostream>
#include <fstream>

#include "elfin_types.h"
#include "input/arg_parser.h"
#include "../jutil/src/jutil.h"
#include "../input/SpecParser.h"
#include "../input/CSVParser.h"
#include "../input/JSONParser.h"
#include "../core/EvolutionSolver.h"
#include "../core/ParallelUtils.h"
#include "../core/MathUtils.h"
#include "../core/Kabsch.h"

#ifndef _NO_OMP
#include <omp.h>
#endif

namespace elfin {

std::vector<ElfinRunner *> ElfinRunner::instances_;

void ElfinRunner::interrupt_handler(const int signal) {
    raw("\n\n");
    wrn("Caught interrupt signal\n");

    // Save latest results
    for (auto er : instances_) {
        er->crash_dump();
        delete er;
    }

    exit(signal);
}

void ElfinRunner::crash_dump() {
    if (es_started_) {
        wrn("Saving latest best solutions and exiting!\n");

        const Population & p = es_->bestSoFar();

        for (int i = 0; i < p.size(); i++) {
            std::vector<std::string> node_names = p.at(i).getNodeNames();
            JSON nn = node_names;
            JSON j;
            j["nodes"] = nn;
            j["score"] = p.at(i).getScore();

            mkdir_ifn_exists("crash_dump");

            std::ostringstream ss;
            ss << "crash_dump/" << &p.at(i) << ".json";
            std::string dump = j.dump();
            const char * data = dump.c_str();
            const size_t len = dump.size();
            write_binary(ss.str().c_str(), data, len);
        }

        delete es_;
    } else {
        wrn("GA did not get to start\n");
    }
}

ElfinRunner::ElfinRunner(const int argc, const char ** argv) {
    instances_.push_back(this);

    std::signal(SIGINT, interrupt_handler);

    // Default set to warning and above
    set_log_level(LOG_WARN);

    ArgParser ap(argc, argv);
    options_ = ap.get_options();

    mkdir_ifn_exists(options_.outputDir.c_str());

    msg("Using master seed: %d\n", options_.randSeed);

    JSONParser().parseDB(options_.xdb,
        name_id_map_,
        id_name_map_,
        rela_mat_,
        radii_list_);

    Gene::setup(&id_name_map_);
    setupParaUtils(options_.randSeed);

    spec_ = ap.get_spec();
}

void ElfinRunner::run() {
    if (options_.runUnitTests) {
        int fail_count = 0;
        fail_count += run_unit_tests();
        fail_count += run_meta_tests();

        if (fail_count > 0) {
            die("Some unit tests failed\n");
        } else {
            msg("Passed!\n");
        }
    } else {
        es_ = new EvolutionSolver(rela_mat_,
                                 spec_,
                                 radii_list_,
                                 options_);
        es_started_ = true;

        es_->run();

        const Population * p = es_->population();

        for (int i = 0; i < options_.nBestSols; i++) {
            std::vector<std::string> node_names = p->at(i).getNodeNames();
            JSON nn = node_names;
            JSON j;
            j["nodes"] = nn;
            j["score"] = p->at(i).getScore();


            // write json solution data (no coordinates)
            std::ostringstream json_output_path;
            json_output_path << options_.outputDir
                << "/" << &p->at(i) << ".json";

            std::string json_data = j.dump();
            write_binary(json_output_path.str().c_str(),
                         json_data.c_str(),
                         json_data.size());

            // write csv (coorindates only)
            std::ostringstream csv_output_path;
            csv_output_path << options_.outputDir << "/" << &p->at(i) << ".csv";

            std::string csv_data = p->at(i).toCSVString();
            write_binary(csv_output_path.str().c_str(),
                         csv_data.c_str(),
                         csv_data.size());
        }

        delete es_;
    }
}

int ElfinRunner::run_unit_tests() {
    msg("Running unit tests...\n");
    int fail_count = 0;
    fail_count += _testMathUtils();
    fail_count += _testKabsch(options_);
    fail_count += _testChromosome(options_);
    return fail_count;
}

int ElfinRunner::run_meta_tests() {
    msg("Running meta tests...\n");
    int fail_count = 0;

    Points3f moved_spec = spec_;

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

    for (Point3f &p : moved_spec) {
        p = p.dot(rot_around_x);
        p += tran;
    }

    // Test scoring a transformed version of spec
    const float trx_score = kabschScore(moved_spec, spec_);
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
    for (int i = 0; i < rand_trials; i++) {
        const int dice = getDice(N);
        if (dice >= N) {
            fail_count++;
            err("Failed to produce correct dice: getDice() "
                "produced %d for [0-%d)",
                dice, N);
            break;
        }
        rand_count[dice]++;
    }

    for (int i = 0; i < N; i++) {
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
    std::vector<uint> para_rand_seeds = getParaRandSeeds();
    const int n_threads = para_rand_seeds.size();
    const int para_rand_n = 8096;
    const int64_t dice_lim = 13377331;

    std::vector<uint> rands1(para_rand_n);
    #pragma omp parallel for
    for (int i = 0; i < para_rand_n; i++)
        rands1.at(i) = getDice(dice_lim);

    getParaRandSeeds() = para_rand_seeds;
    std::vector<uint> rands2(para_rand_n);
    #pragma omp parallel for
    for (int i = 0; i < para_rand_n; i++)
        rands2.at(i) = getDice(dice_lim);

    for (int i = 0; i < para_rand_n; i++) {
        if (rands1.at(i) != rands2.at(i)) {
            fail_count++;
            err("Parallel randomiser failed: %d vs %d\n",
                rands1.at(i), rands2.at(i));
        }
    }
#endif

    return fail_count;
}
}  // namespace elfin

int main(const int argc, const char ** argv) {
    elfin::ElfinRunner(argc, argv).run();
    return 0;
}
