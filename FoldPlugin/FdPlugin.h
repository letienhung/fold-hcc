#pragma once

#include "interfaces/ModePluginDockWidget.h"
#include "SurfaceMeshPlugins.h"
#include "GraphManager.h"
#include "FoldManager.h"
#include <QColorDialog>

class FdWidget;

class FdPlugin : public SurfaceMeshModePlugin
{
	Q_OBJECT
	Q_INTERFACES(ModePlugin)

	// Plug-in interfaces
    QIcon icon(){ return QIcon(":/images/icon.png"); }
	void create();
	void destroy();
	void decorate();

	void drawWithNames();
	bool postSelection(const QPoint& point);

	// Keyboard / mouse
	bool keyPressEvent(QKeyEvent* event);

public:
	FdWidget* widget;
	GraphManager* g_manager;
	FoldManager* f_manager;

	bool drawAABB;
	bool drawCuboid;
	bool drawScaffold;
	bool drawMesh;
	bool drawFolded;
	bool drawKeyframe;
	bool drawAFS;
	bool drawNodeOrder;

	QColorDialog* qColorDialog;
public:
    FdPlugin();
	
	// helpers
	FdGraph* activeScaffold();
	QVector<FdNode*> selectedFdNodes();

public slots:
	// to graph manager
	void resetMesh();

	// show options
	void showAABB(int state);
	void showCuboid(int state);
	void showScaffold(int state);
	void showMesh(int state);
	void showFolded(int state);
	void showKeyframe(int state);
	void showAFS(int state);

	// scene and message
	void updateScene();
	void resetScene();
	void showStatus(QString msg);

	// export
	void exportCurrent();

	// color dialog
	void showColorDialog();
	void updateSelNodesColor(QColor c);
	void colorMasterSlave();

	// snapshot
	void saveSnapshot();
	void saveSnapshotAll();
	void exportSVG();

	// hide
	void hideSelectedNodes();
	void unhideAllNodes();
	void hideEdgeRods();

	// debug
	void test1();
	void test2();
};


