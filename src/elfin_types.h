#ifndef TYPE_DEFS_H
#define TYPE_DEFS_H

#include <vector>
#include <map>
#include <sstream>
#include <string>
#include <tuple>

#include "../data/Geometry.h"

#define toCString toString().c_str

namespace elfin {

class PairRelationship;

// Shorthands
typedef std::map<std::string, long> NameIdMap;
typedef std::map<long, std::string> IdNameMap;

typedef std::vector<PairRelationship *> RelaRow;
typedef std::vector<RelaRow> RelaMat;

typedef std::vector<long> IdRoulette;
typedef std::vector<long> Ids;

typedef uint32_t uint;
typedef uint64_t ulong;

struct Vec2d {
	long x, y;
	Vec2d() : x(0), y(0) {};
	Vec2d(long _x, long _y) : x(_x), y(_y) {};
};

typedef Vec2d IdPair;
typedef std::vector<IdPair> IdPairs;
typedef std::vector<std::tuple<IdPair, IdPairs>> CrossingVector;

template <typename T>
using Matrix = std::vector<std::vector<T>>;

struct Radii {
	float avgAll;
	float maxCA;
	float maxHeavy;
	Radii(float aa, float mca, float mh) :
		avgAll(aa), maxCA(mca), maxHeavy(mh)
	{}
};
typedef std::vector<Radii> RadiiList;

typedef struct {
	bool valid = true;

	// Input settings
	std::string xdb = "xDB.json";
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
} Options;

} // namespace elfin

#endif /* include guard */
