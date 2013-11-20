#pragma once

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

	QVector<double>		jumpTypeProbability;		// Probability of proposed jump types
	double				useActiveHingeProbability;
	double				useHotProbability;
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

