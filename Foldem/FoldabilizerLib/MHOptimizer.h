#pragma once
#include "FoldabilizerLibGlobal.h"
#include "Graph.h"

class MHOptimizer
{
public:
	Graph* hccGraph;
    MHOptimizer(Graph *graph);

	bool				isReady;
	double				originalAabbVolume;			// original function volume
	double				originalMaterialVolume;
	QVector<double>		typeProbability;			//Proposal distributions
	GraphState			currState;
	double				currCost;
	void initialize();

	void	jump();
	double	cost();
	void	proposeJump();
	bool	doesAcceptJump();
};

