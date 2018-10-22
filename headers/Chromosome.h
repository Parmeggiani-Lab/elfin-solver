#ifndef CHROMOSOME_H
#define CHROMOSOME_H

#include <cmath>
#include <string>

#include "options.h"
#include "id_pair.h"
#include "radii.h"
#include "checksum.h"
#include "jutil.h"

#include "Gene.h"

namespace elfin
{

// Some of the stochastic processes may fail
// to meet algorithm criteria
#define MAX_STOCHASTIC_FAILS 10

#define FOREACH_ORIGIN(v) \
		v(New) \
		v(Copy) \
		v(GeneCopy) \
		v(AutoMutate) \
		v(Cross) \
		v(PointMutate) \
		v(LimbMutate) \
		v(Random)

GEN_ENUM_AND_STRING(Origin, OriginString, FOREACH_ORIGIN);

class Chromosome
{
public:
	/* ctors */
	Chromosome();
	Chromosome(const Chromosome & rhs);
	Chromosome(const Genes & genes);

	/* operators */
	bool operator>(const Chromosome & rhs) const;
	bool operator<(const Chromosome & rhs) const;

	/* getters */
	float getScore() const;
	const Genes & genes() const;
	Crc32 checksum() const;
	std::vector<std::string> getNodeNames() const;
	static int min_len() { return min_len_; }
	static int max_len() { return max_len_; }

	/* setters */
	Genes & genes();

	void score(const Points3f & ref);
	bool cross(const Chromosome & father, Chromosome & out) const;
	void autoMutate();
	void randomise();
	bool pointMutate();
	bool limbMutate();
	void setOrigin(Origin o);
	Origin getOrigin() const;
	Chromosome copy() const;
	std::string to_string() const;
	std::string to_csv_string() const;

	static Genes genRandomGenesReverse(
	    const uint genMaxLen=max_len_,
	    Genes genes=Genes());
	static Genes genRandomGenes(
	    const uint genMaxLen=max_len_,
	    Genes genes=Genes());

	static void setup(const int exp_len,
	                  const int len_dev,
	                  const RelaMat & rela_mat,
	                  const RadiiList & radii_list);
	static uint calcExpectedLength(const Points3f & len_ref,
	                               const float avg_pair_dist);
	static bool synthesiseReverse(Genes & genes);
	static bool synthesise(Genes & genes);

private:
	Genes myGenes;
	float myScore = NAN;
	Origin myOrigin = Origin::New;

	static bool setupDone;
	static int min_len_;
	static int max_len_;
	static const RelaMat * myRelaMat;
	static const RadiiList * myRadiiList;
	static IdPairs myNeighbourCounts;
	static IdRoulette myGlobalRoulette;
};

int _testChromosome(const Options &options);
} // namespace elfin

#endif /* include guard */
