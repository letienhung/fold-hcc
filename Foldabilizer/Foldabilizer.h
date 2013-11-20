#pragma once

#include "interfaces/ModePluginDockWidget.h"
#include "SurfaceMeshPlugins.h"

#include "Graph.h"
#include "MHOptimizer.h"

class foldem_widget;

class Foldabilizer : public SurfaceMeshModePlugin
{
	Q_OBJECT
	Q_INTERFACES(ModePlugin)

	// Plugin interfaces
	QIcon icon(){ return QIcon("images/icon.png"); }
	void create();
	void destroy();
	void decorate();

	void resetScene();

public:
    Foldabilizer();

	Graph			*hccGraph;
	MHOptimizer		*mhOptimizer;
	foldem_widget	*widget;

	int	stepsPerJump;

public slots:
	void createI();
	void createL();
	void createT();
	void createX();
	void createSharp();
	void createU();
	void createO();
	void createChair(double legL = 1.5);
	void loadGraph();
	void jump();
	void test();

	void setTargetVolumePercentage(int v);
	void setLinkProbability(double lp);
	void setCostWeight(double weight);
	void setTemprature(int T);
	void setStepsPerJump(int steps);
};


