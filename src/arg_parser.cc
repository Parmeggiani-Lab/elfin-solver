#include "arg_parser.h"

#include <string>
#include <regex>
#include <sstream>

#include "../input/CSVParser.h"
#include "../input/JSONParser.h"

/* 
    Since jutil is in C and it only accepts C style function pointers, I can't
    use class methods with DECL_ARG_CALLBACK. To access a persistent
    OptionPack, I'd need it to be a private variable. But I can't use methods,
    so the closest C alternative is a file-scope static.

    Maybe this is stupid and I should just do jutil in C++.
*/
static elfin::OptionPack ap_options;

DECL_ARG_CALLBACK(helpAndExit); // defined later due to use of argb size
DECL_ARG_CALLBACK(setConfigFile) { ap_options.configFile = arg_in; }
DECL_ARG_CALLBACK(setInputFile) { ap_options.inputFile = arg_in; }
DECL_ARG_CALLBACK(setXDB) { ap_options.xdb = arg_in; }
DECL_ARG_CALLBACK(setOutputDir) { ap_options.outputDir = arg_in; }

DECL_ARG_CALLBACK(setLenDevAlw) { ap_options.lenDevAlw = parse_long(arg_in); }
DECL_ARG_CALLBACK(setAvgPairDist) { ap_options.avgPairDist = parse_float(arg_in); }
DECL_ARG_CALLBACK(setRandSeed) { ap_options.randSeed = parse_long(arg_in); }

DECL_ARG_CALLBACK(setGaPopSize) { ap_options.gaPopSize = parse_long(arg_in); }
DECL_ARG_CALLBACK(setGaIters) { ap_options.gaIters = parse_long(arg_in); }
DECL_ARG_CALLBACK(setGaSurviveRate) { ap_options.gaSurviveRate = parse_float(arg_in); }
DECL_ARG_CALLBACK(setGaCrossRate) { ap_options.gaCrossRate = parse_float(arg_in); }
DECL_ARG_CALLBACK(setGaPointMutateRate) { ap_options.gaPointMutateRate = parse_float(arg_in); }
DECL_ARG_CALLBACK(setGaLimbMutateRate) { ap_options.gaLimbMutateRate = parse_float(arg_in); }
DECL_ARG_CALLBACK(setScoreStopThreshold) { ap_options.scoreStopThreshold = parse_float(arg_in); }
DECL_ARG_CALLBACK(setMaxStagnantGens) { ap_options.maxStagnantGens = parse_long(arg_in); }
DECL_ARG_CALLBACK(setLogLevel) { set_log_level((Log_Level) parse_long(arg_in)); }
DECL_ARG_CALLBACK(setRunUnitTests) { ap_options.runUnitTests = true; }
DECL_ARG_CALLBACK(setDevice) { ap_options.device = parse_long(arg_in); }
DECL_ARG_CALLBACK(setNBestSols) { ap_options.nBestSols = parse_long(arg_in); }

const argument_bundle argb[] = {
    {"h", "help", "Print this help text and exit", false, helpAndExit},
    {"c", "setConfigFile", "Set config file (default ./config.json)", true, setConfigFile},
    {"i", "inputFile", "Set input file", true, setInputFile},
    {"x", "xdb", "Set xDB file (default ./xDB.json)", true, setXDB},
    {"o", "outputDir", "Set output directory (default ./out/)", true, setOutputDir},
    {"d", "lenDevAlw", "Set length deviation allowance (default 3)", true, setLenDevAlw},
    {"a", "avgPairDist", "Overwrite default average distance between doubles of CoMs (default 38.0)", true, setAvgPairDist},
    {"rs", "randSeed", "Set RNG seed (default 0x1337cafe; setting to 0 uses time as seed)", true, setRandSeed},
    {"gps", "gaPopSize", "Set GA population size (default 10000)", true, setGaPopSize},
    {"git", "gaIters", "Set GA iterations (default 1000)", true, setGaIters},
    {"gsr", "gaSurviveRate", "Set GA survival rate (default 0.1)", true, setGaSurviveRate},
    {"gcr", "gaCrossRate", "Set GA surviver cross rate (default 0.60)", true, setGaCrossRate},
    {"gmr", "gaPointMutateRate", "Set GA surviver point mutation rate (default 0.3)", true, setGaPointMutateRate},
    {"gmr", "gaLimbMutateRate", "Set GA surviver limb mutation rate (default 0.3)", true, setGaLimbMutateRate},
    {"stt", "scoreStopThreshold", "Set GA exit score threshold (default 0.0)", true, setScoreStopThreshold},
    {"msg", "maxStagnantGens", "Set number of stagnant generations before GA exits (default 50)", true, setMaxStagnantGens},
    {"lg", "logLevel", "Set log level", true, setLogLevel},
    {"t", "test", "Run unit tests", false, setRunUnitTests},
    {"dv", "device", "Run on accelerator device ID", true, setDevice},
    {"n", "nBestSols", "Set number of best solutions to output", true, setNBestSols}
};

DECL_ARG_CALLBACK(helpAndExit) {
    raw("elfin: An Automatic Protein Designer\n");
    raw("Usage: ./elfin [OPTIONS]\n");
    raw("Note: Setting an option will override the same setting in the json file\n");

    print_arg_bundles(argb);

    exit(1);
}

DECL_ARG_IN_FAIL_CALLBACK(argParseFail) {
    printf("Argument parsing failed on string: \"%s\"\n", arg_in);
    helpAndExit(nullptr);
    exit(1);
}

namespace elfin {

void parse_settings() {
    panic_if(!file_exists(ap_options.configFile.c_str()),
             "Settings file does not exist: \"%s\"\n",
             ap_options.configFile.c_str());

    JSON j = JSONParser().parse(ap_options.configFile);

    std::string tmpStr;
    auto jsonToCStr = [&](const JSON & j) {
        std::ostringstream ss;
        if (j.is_string())
            ss << j.get<std::string>();
        else
            ss << j;
        tmpStr = ss.str();
        return tmpStr.c_str();
    };

    const size_t arg_bund_size = (sizeof(argb) / sizeof(argb[0]));
    for (int i = 0; i < arg_bund_size; i++)
    {
        const argument_bundle * ab = &argb[i];
        if (!j[ab->long_form].is_null())
            ab->call_back(jsonToCStr(j[ab->long_form]));
    }
}

void check_options() {
    // Do basic checks for each option

    // Files
    panic_if(ap_options.xdb == "",
             "No xDB file given. Check your settings.json\n");

    panic_if(!file_exists(ap_options.xdb.c_str()),
             "xDB file could not be found\n");

    panic_if(ap_options.inputFile == "",
             "No input spec file given. Check help using -h\n");

    panic_if(!file_exists(ap_options.inputFile.c_str()),
             "Input file could not be found\n");

    panic_if(ap_options.configFile == "",
             "No settings file file given. Check help using -h\n");

    panic_if(!file_exists(ap_options.configFile.c_str()),
             "Settings file \"%s\" could not be found\n",
             ap_options.configFile.c_str());

    panic_if(ap_options.outputDir == "",
             "No output directory given. Check help using -h\n");

    if (!file_exists(ap_options.outputDir.c_str()))
    {
        wrn("Output directory does not exist; creating...\n");
        mkdir_ifn_exists(ap_options.outputDir.c_str());
    }

    // Extensions
    if (std::regex_match(
                ap_options.inputFile,
                std::regex("(.*)(\\.csv)", std::regex::icase)))
    {
        msg("Using CSV input\n");
        ap_options.inputType = OptionPack::InputType::CSV;
    }
    else if (std::regex_match(
                 ap_options.inputFile,
                 std::regex("(.*)(\\.json)", std::regex::icase)))
    {
        msg("Using JSON input\n");
        ap_options.inputType = OptionPack::InputType::JSON;
    }
    else {
        die("Unrecognized input file type\n");
    }

    // Settings

    panic_if(ap_options.gaPopSize < 0, "Population size cannot be < 0\n");

    panic_if(ap_options.gaIters < 0, "Number of iterations cannot be < 0\n");

    panic_if(ap_options.lenDevAlw < 0,
             "Gene length deviation must be an integer > 0\n");

    // GA params
    panic_if(ap_options.gaSurviveRate < 0.0 ||
             ap_options.gaSurviveRate > 1.0,
             "GA survive rate must be between 0 and 1 inclusive\n");
    panic_if(ap_options.gaCrossRate < 0.0 ||
             ap_options.gaCrossRate > 1.0,
             "GA cross rate must be between 0 and 1 inclusive\n");
    panic_if(ap_options.gaPointMutateRate < 0.0 ||
             ap_options.gaPointMutateRate > 1.0,
             "GA point mutate rate must be between 0 and 1 inclusive\n");
    panic_if(ap_options.gaLimbMutateRate < 0.0 ||
             ap_options.gaLimbMutateRate > 1.0,
             "GA limb mutate rate must be between 0 and 1 inclusive\n");

    bool rateCorrected = false;
    float sumRates = ap_options.gaCrossRate +
                     ap_options.gaPointMutateRate +
                     ap_options.gaLimbMutateRate;
    if ((rateCorrected = (sumRates > 1.0)))
    {
        ap_options.gaCrossRate         /= sumRates;
        ap_options.gaPointMutateRate   /= sumRates;
        ap_options.gaLimbMutateRate    /= sumRates;
        sumRates = ap_options.gaCrossRate +
                   ap_options.gaPointMutateRate +
                   ap_options.gaLimbMutateRate;
    }

    if (rateCorrected)
    {
        wrn("Sum of GA cross + point mutate + limb mutate rates must be <= 1\n");
        wrn("Rates corrected to: %.2f, %.2f, %.2f\n",
            ap_options.gaCrossRate,
            ap_options.gaPointMutateRate,
            ap_options.gaLimbMutateRate);
    }

    panic_if(ap_options.avgPairDist < 0, "Average CoM distance must be > 0\n");

    panic_if(ap_options.nBestSols < 0 ||
             ap_options.nBestSols > ap_options.gaPopSize,
             "Number of best solutions to output must be > 0 and < gaPopSize\n");
}

Points3f ArgParser::parse_input(const OptionPack & options) const {
    switch (options.inputType)
    {
    case OptionPack::InputType::CSV:
        return CSVParser().parseSpec(options.inputFile);
    case OptionPack::InputType::JSON:
        return JSONParser().parseSpec(options.inputFile);
    default:
        die("Unknown input format\n");
    }

    return Points3f();
}

OptionPack ArgParser::parse(const int argc, char const *argv[]) const {
    // Parse user arguments first to potentially get a settings file path
    parse_args(argc, argv, argb, argParseFail);

    // Get values from settings json config
    parse_settings();

    // Parse user arguments a second time to override settings file
    parse_args(argc, argv, argb, argParseFail);

    check_options();

    return ap_options;
}

} // namespace elfin