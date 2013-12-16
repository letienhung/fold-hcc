#pragma once

#include "interfaces/ModePluginDockWidget.h"
#include "SurfaceMeshPlugins.h"
#include "GraphManager.h"
#include "Foldabilizer.h"

class FdWidget;

class FdPlugin : public SurfaceMeshModePlugin
{
	Q_OBJECT
	Q_INTERFACES(ModePlugin)

	// Plugin interfaces
    QIcon icon(){ return QIcon(":/images/icon.png"); }
	void create();
	void destroy();
	void decorate();

	void drawWithNames();
	bool postSelection(const QPoint& point);

public:
    FdPlugin();
    FdWidget	*widget;
	GraphManager* g_manager;

	FdGraph* activeScaffold();

public slots:
	void updateScene();
	void resetScene();

	void resetMesh();
	void showStatus(QString msg);

	void fold();

	void test();
};


