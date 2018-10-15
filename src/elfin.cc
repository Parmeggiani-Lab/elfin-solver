#include <string>
#include <regex>
#include <sstream>
#include <csignal>
#include <iostream>
#include <fstream>

#include "../data/TypeDefs.h"
#include "jutil.h"
#include "../input/SpecParser.h"
#include "../input/CSVParser.h"
#include "../input/JSONParser.h"
#include "../core/EvolutionSolver.h"
#include "../core/ParallelUtils.h"
#include "../core/MathUtils.h"
#include "../core/Kabsch.h"
#include "arg_parser.h"

#ifndef _NO_OMP
#include <omp.h>
#endif

namespace elfin
{

OptionPack options;

EvolutionSolver * es;
bool esStarted = false;

void interruptHandler(int signal)
{
    raw("\n\n");
    wrn("Caught interrupt signal\n");

    // Save latest results
    if (esStarted)
    {
        wrn("Saving latest best solutions and exiting!\n");
        using namespace elfin;

        const Population & p = es->bestSoFar();

        for (int i = 0; i < p.size(); i++)
        {
            std::vector<std::string> nodeNames = p.at(i).getNodeNames();
            JSON nn = nodeNames;
            JSON j;
            j["nodes"] = nn;
            j["score"] = p.at(i).getScore();

            std::ostringstream ss;
            ss << options.outputDir << "/" << &p.at(i) << ".json";
            std::string dump = j.dump();
            const char * data = dump.c_str();
            const size_t len = dump.size();
            write_binary(ss.str().c_str(), data, len);
        }

        delete es;
    }
    else
    {
        wrn("GA did not get to start\n");
    }

    exit(1);
}

int runUnitTests()
{
    msg("Running unit tests...\n");
    int failCount = 0;
    failCount += _testMathUtils();
    failCount += _testKabsch();
    failCount += _testChromosome();
    return failCount;
}

int runMetaTests(const Points3f & spec)
{
    msg("Running meta tests...\n");
    int failCount = 0;

    Points3f movedSpec = spec;

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
    const float trxScore = kabschScore(movedSpec, spec);
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

int main(int argc, const char ** argv)
{
    std::signal(SIGINT, interruptHandler);

    // Default set to warning and above
    set_log_level(LOG_WARN);

    auto ap = ArgParser();
    elfin::options = ap.parse(argc, argv);

    mkdir_ifn_exists(options.outputDir.c_str());

    msg("Using master seed: %d\n", options.randSeed);

    RelaMat relaMat;
    NameIdMap nameIdMap;
    IdNameMap idNameMap;
    RadiiList radiiList;
    JSONParser().parseDB(options.xdb, nameIdMap, idNameMap, relaMat, radiiList);

    Gene::setup(&idNameMap);
    setupParaUtils(options.randSeed);

    Points3f spec = ap.parse_input(elfin::options);

    if (options.runUnitTests)
    {
        int failCount = 0;
        failCount += runUnitTests();
        failCount += runMetaTests(spec);

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
        es = new EvolutionSolver(relaMat,
                                 spec,
                                 radiiList,
                                 options);
        esStarted = true;

        es->run();

        const Population * p = es->population();

        for (int i = 0; i < options.nBestSols; i++)
        {
            std::vector<std::string> nodeNames = p->at(i).getNodeNames();
            JSON nn = nodeNames;
            JSON j;
            j["nodes"] = nn;
            j["score"] = p->at(i).getScore();


            // write json solution data (no coordinates)
            std::ostringstream jsonOutputPath;
            jsonOutputPath << options.outputDir << "/" << &p->at(i) << ".json";

            std::string dump = j.dump();
            write_binary(jsonOutputPath.str().c_str(),
                         dump.c_str(),
                         dump.size());

            // write csv (coorindates only)
            std::ostringstream csvOutputPath;
            csvOutputPath << options.outputDir << "/" << &p->at(i) << ".csv";

            std::string csvData = p->at(i).toCSVString();
            write_binary(csvOutputPath.str().c_str(),
                         csvData.c_str(),
                         csvData.size());
        }

        delete es;
    }

    return 0;
}
