#include <string>
#include <regex>
#include <sstream>
#include <csignal>
#include <iostream>
#include <fstream>

#include "elfin.h"
#include "elfin_types.h"
#include "arg_parser.h"
#include "jutil.h"
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

namespace elfin
{

std::vector<ElfinRunner *> ElfinRunner::instances_;

void ElfinRunner::interrupt_handler(const int signal)
{
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
    if (es_started_)
    {
        wrn("Saving latest best solutions and exiting!\n");
        using namespace elfin;

        const Population & p = es_->bestSoFar();

        for (int i = 0; i < p.size(); i++)
        {
            std::vector<std::string> nodeNames = p.at(i).getNodeNames();
            JSON nn = nodeNames;
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
    }
    else
    {
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

    JSONParser().parseDB(options_.xdb, name_id_map_, id_name_map_, rela_mat_, radii_list_);

    Gene::setup(&id_name_map_);
    setupParaUtils(options_.randSeed);

    spec_ = ap.get_spec();
}

void ElfinRunner::run() {
    if (options_.runUnitTests)
    {
        int failCount = 0;
        failCount += run_unit_tests();
        failCount += run_meta_tests();

        if (failCount > 0)
        {
            die("Some unit tests failed\n");
        }
        else
        {
            msg("Passed!\n");
        }
    }
    else
    {
        es_ = new EvolutionSolver(rela_mat_,
                                 spec_,
                                 radii_list_,
                                 options_);
        es_started_ = true;

        es_->run();

        const Population * p = es_->population();

        for (int i = 0; i < options_.nBestSols; i++)
        {
            std::vector<std::string> nodeNames = p->at(i).getNodeNames();
            JSON nn = nodeNames;
            JSON j;
            j["nodes"] = nn;
            j["score"] = p->at(i).getScore();


            // write json solution data (no coordinates)
            std::ostringstream jsonOutputPath;
            jsonOutputPath << options_.outputDir << "/" << &p->at(i) << ".json";

            std::string dump = j.dump();
            write_binary(jsonOutputPath.str().c_str(),
                         dump.c_str(),
                         dump.size());

            // write csv (coorindates only)
            std::ostringstream csvOutputPath;
            csvOutputPath << options_.outputDir << "/" << &p->at(i) << ".csv";

            std::string csvData = p->at(i).toCSVString();
            write_binary(csvOutputPath.str().c_str(),
                         csvData.c_str(),
                         csvData.size());
        }

        delete es_;
    }
}

int ElfinRunner::run_unit_tests()
{
    msg("Running unit tests...\n");
    int failCount = 0;
    failCount += _testMathUtils();
    failCount += _testKabsch(options_);
    failCount += _testChromosome(options_);
    return failCount;
}

int ElfinRunner::run_meta_tests()
{
    msg("Running meta tests...\n");
    int failCount = 0;

    Points3f movedSpec = spec_;

    Vector3f rotArr[3] = {
        Vector3f(1.0f, 0.0f, 0.0f),
        Vector3f(0.0f, -0.5177697998f, 0.855519979f),
        Vector3f(0.0f, -0.855519979f, -0.5177697998f)
    };
    Mat3x3 rotAroundX = Mat3x3(rotArr);

    // It seems that Kabsch cannot handle very large
    // translations, but in the scale we're working
    // at it should rarely, if ever, go beyond
    // one thousand Angstroms
    Vector3f tran(-39.0f, 999.3413f, -400.11f);

    for (Point3f &p : movedSpec)
    {
        p = p.dot(rotAroundX);
        p += tran;
    }

    // Test scoring a transformed version of spec
    const float trxScore = kabschScore(movedSpec, spec_);
    if (!float_approximates(trxScore, 0))
    {
        failCount ++;
        wrn("Self score test failed: self score should be 0\n");
    }

    // Test randomiser
    const int N = 10;
    const int randTrials = 50000000;
    const int expectAvg = randTrials / N;
    const float randDevTolerance = 0.05f * expectAvg; //5% deviation

    int randCount[N] = {0};
    for (int i = 0; i < randTrials; i++)
    {
        const int dice = getDice(N);
        if (dice >= N)
        {
            failCount++;
            err("Failed to produce correct dice: getDice() produced %d for [0-%d)",
                dice, N);
            break;
        }
        randCount[dice]++;
    }

    for (int i = 0; i < N; i++)
    {
        const float randDev = (float) abs(randCount[i] - expectAvg) / (expectAvg);
        if (randDev > randDevTolerance)
        {
            failCount++;
            err("Too much random deviation: %.3f%% (expecting %d)\n",
                randDev, expectAvg);
        }
    }

    // Test parallel randomiser
#ifndef _NO_OMP
    std::vector<uint> paraRandSeeds = getParaRandSeeds();
    const int nThreads = paraRandSeeds.size();
    const int paraRandN = 8096;
    const long diceLim = 13377331;

    std::vector<uint> rands1(paraRandN);
    #pragma omp parallel for
    for (int i = 0; i < paraRandN; i++)
        rands1.at(i) = getDice(diceLim);

    getParaRandSeeds() = paraRandSeeds;
    std::vector<uint> rands2(paraRandN);
    #pragma omp parallel for
    for (int i = 0; i < paraRandN; i++)
        rands2.at(i) = getDice(diceLim);

    for (int i = 0; i < paraRandN; i++)
    {
        if (rands1.at(i) != rands2.at(i))
        {
            failCount++;
            err("Parallel randomiser failed: %d vs %d\n",
                rands1.at(i), rands2.at(i));
        }
    }
#endif

    return failCount;
}
} // namespace elfin

using namespace elfin;

/*
 * The elfin design process:
 *   Input:
 *      A vector of Centre of Mass shape specification
 *   Algorithm:
 *      GA with a variety of inheritance and also a desturctive
 *      gene (shape candidate) operator
 *   Output:
 *      A vector of module (node) names suitable for
 *      use by Synth.py to produce full PDB
 */

int main(const int argc, const char ** argv)
{
    ElfinRunner(argc, argv).run();
    return 0;
}
