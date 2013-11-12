#pragma once
#include "FoldabilizerLibGlobal.h"
#include "Graph.h"
#include "Numeric.h"

class MHOptimizer
{
public:
	Graph* hccGraph;
    MHOptimizer(Graph *graph);

	bool				isReady;
	double				originalAabbVolume;			// Original function volume
	double				originalMaterialVolume;
	GraphState			currState;
	double				currCost;
	QVector<double>		typeProbability;			// Probability of proposed jump types
	NormalDistribution	normalDistribution;
	void initialize();

	void	jump();
	double	cost();
	void	proposeJump();
	bool	acceptJump();
	bool	isCollisionFree();
};

