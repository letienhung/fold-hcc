#pragma once

#include "interfaces/ModePluginDockWidget.h"
#include "SurfaceMeshPlugins.h"

#include "HccManager.h"
#include "MHOptimizer.h"

class FoldabilizerWidget;

class Foldabilizer : public SurfaceMeshModePlugin
{
	Q_OBJECT
	Q_INTERFACES(ModePlugin)

	// Plugin interfaces
    QIcon icon(){ return QIcon(":/images/icon.png"); }
	void create();
	void destroy();
	void decorate();


public:
    Foldabilizer();

	HccManager		*hccManager;
	MHOptimizer		*mhOptimizer;
    FoldabilizerWidget	*widget;

	HccGraph* activeHcc();

public slots:
	void updateScene();
	void resetScene();
	void loadGraph();
	void test();
};


