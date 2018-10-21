#ifndef OPTIONS_H_
#define OPTIONS_H_

#include <string>

namespace elfin {

typedef struct {
    bool valid = true;

    // Input settings
    std::string xdb = "xdb.json";
    std::string inputFile = "";

    std::string configFile = "";
    std::string outputDir = "output";

    ulong lenDevAlw = 3;

    // Average CoM distance found by xDBStat.py as
    // of 23/April/2017 is 37.9
    float avgPairDist = 38.0f;

    // GA parameters
    uint randSeed = 0x1337cafe;
    long gaPopSize = 10000;
    long gaIters = 1000;
    float gaSurviveRate = 0.1f;
    float gaCrossRate = 0.5f;
    float gaPointMutateRate = 0.5f;
    float gaLimbMutateRate = 0.5f;

    // Use a small number but not exactly 0.0
    // because of imprecise float comparison
    float scoreStopThreshold = 0.01f;

    int maxStagnantGens = 50;

    bool runUnitTests = false;

    int device = 0;
    int nBestSols = 3;

    bool dry_run = false;
} Options;

}  /* elfin */

#endif  /* end of include guard: OPTIONS_H_ */