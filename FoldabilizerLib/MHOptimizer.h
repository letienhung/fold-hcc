#pragma once
#include <QObject>

#include "HccManager.h"
#include "ProbabilityDistributions.h"

class MHOptimizer : public QObject
{
	Q_OBJECT

public:
	HccManager* hccManager;
    MHOptimizer(HccManager *hccM);
	HccGraph* activeHcc();

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
	void	resolveCollision(Node* pn);
	bool	acceptJump();

	// helper
	double	cost();

public slots:
	void	jump(int steps);
	void	jump();
	void	debug();

signals:
	void	hccChanged();
};

