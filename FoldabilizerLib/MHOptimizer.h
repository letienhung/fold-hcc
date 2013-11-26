#pragma once

#include "Graph.h"
#include "ProbabilityDistributions.h"

class MHOptimizer
{
public:
	Graph* hccGraph;
    MHOptimizer(Graph *graph);


	bool				isReady;
	double				originalAabbVolume;	
	double				originalMaterialVolume;
	GraphState			currState;
	double				currCost;

	// proposal
	NormalDistribution	normalDistr;
	int					nbSigma;		// default is 6, 6-sigma covers 99.7%
	QVector<double>		typeProb;
	double				switchHingeProb;
	double				useHotProb;

	// acceptance
	double	distWeight;
	int		temperature;

	// target
	double	targetV;
	int		jumpCount;

	void initialize();

	void	jump();
	double	cost();
	void	proposeJump();
	void	proposeChangeHingeAngle();
	void	proposeDeformCuboid();
	bool	acceptJump();
	bool	isCollisionFree();
	void	setTypeProb(QVector<double> &tp);
	void	setTypeProb(double t0, double t1);
};

