#pragma once
#include "FoldabilizerLibGlobal.h"
#include "Graph.h"
#include "ProbabilityDistributions.h"

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

	double	targetVolumePercentage;
	double	costWeight;
	int		temperature;
	int		jumpCount;

	void initialize();
	void setLinkProbability(double lp);



	void	jump();
	double	cost();
	void	proposeJump();
	void	proposeChangeHingeAngle();
	void	proposeDeformCuboid();
	bool	acceptJump();
	bool	isCollisionFree();
};

