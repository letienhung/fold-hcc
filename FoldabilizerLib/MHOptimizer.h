#pragma once
#include <QObject>

#include "Graph.h"
#include "ProbabilityDistributions.h"

class MHOptimizer : public QObject
{
	Q_OBJECT

public:
	Graph* hccGraph;
    MHOptimizer(Graph *graph);

	// initialize 
	double		origV;	
	double		origMtlV;
	GraphState	currState;
	double		currCost;
	bool		isReady;
	void		initialize();

	// the proposed hinge
	Hinge*	propHinge;
	double	resCollProb;

	// acceptance
	bool	alwaysAccept;
	double	targetVPerc;
	double	distWeight;
	int		temperature;

	// statistics
	int		jumpCount;

public:
	// optimize
	void	run();
	void	proposeJump();
	void	resolveCollision();
	bool	acceptJump();

	// helper
	double	cost();

	// one single jump
	void	jump();

signals:
	void	hccChanged();
};

