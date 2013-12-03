#pragma once

#include "interfaces/ModePluginDockWidget.h"
#include "SurfaceMeshPlugins.h"

#include "Graph.h"
#include "MHOptimizer.h"

class FoldabilizerWidget;

class Foldabilizer : public SurfaceMeshModePlugin
{
	Q_OBJECT
	Q_INTERFACES(ModePlugin)

	// Plugin interfaces
	QIcon icon(){ return QIcon("images/icon.png"); }
	void create();
	void destroy();
	void decorate();


public:
    Foldabilizer();

	Graph			*hccGraph;
	MHOptimizer		*mhOptimizer;
    FoldabilizerWidget	*widget;

	int	stepsPerJump;

public slots:
	void updateScene();
	void resetScene();
	void createI();
	void createL();
	void createT();
	void createX();
	void createSharp();
	void createU(double uleft, double umid, double uright);
	void createO();
	void createO_2();
	void createBox();
	void createChair(double legL);
	void loadGraph();
	void jump();
	void test();

signals:
	void hccGraphChanged();
};


