#ifndef EVOLUTIONSOLVER_H
#define EVOLUTIONSOLVER_H

#include "shorthands.h"
#include "options.h"
#include "Chromosome.h"
#include "spec.h"

namespace elfin
{

typedef std::vector<Chromosome> Population;

class EvolutionSolver
{
protected:
	const RelaMat & myRelaMat;
	const Spec & mySpec;
	const RadiiList & myRadiiList;
	const Options & myOptions;

	uint myExpectedTargetLen;
	uint myMinTargetLen;
	uint myMaxTargetLen;
	ulong myNonSurviverCount;
	ulong mySurviverCutoff;
	ulong myCrossCutoff;
	ulong myPointMutateCutoff;
	ulong myLimbMutateCutoff;

	double myStartTimeInUs = 0;
	Population myPopulationBuffers[2]; // double buffer
	Chromosome * myBuffPopData;
	const Chromosome * myCurrPopData;
	size_t myPopSize;

	const Population * myCurrPop;
	Population * myBuffPop;
	Population myBestSoFar; // Currently used for emergency output

	double myTotEvolveTime = 0.0f;
	double myTotScoreTime = 0.0f;
	double myTotRankTime = 0.0f;
	double myTotSelectTime = 0.0f;
	double myTotGenTime = 0.0f;
	
	void printStartMsg(const WorkArea & wa);
	void startTimer();
	void set_length_guesses(const WorkArea & wa);

	void initPopulation(const WorkArea & wa);
	void evolvePopulation(const WorkArea & wa);
	void scorePopulation(const WorkArea & wa);
	void rankPopulation();
	void selectParents();
	void swapPopBuffers();

	void printEndMsg();
	void printTiming();

public:
	EvolutionSolver(const RelaMat & relaMat,
	                const Spec & spec,
	                const RadiiList & radiiList,
	                const Options & options);
	virtual ~EvolutionSolver() {};

	const Population * population() const;
	const Population & bestSoFar() const;

	void run();
};

} // namespace elfin

#endif /* include guard */
