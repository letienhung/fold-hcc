#pragma once

#include "interfaces/ModePluginDockWidget.h"
#include "SurfaceMeshPlugins.h"

#include "Graph.h"

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

	foldem_widget *widget;
	Graph *hccGraph;

public slots:
	void createL();
	void createT();
	void createX();
	void createChair();
};


