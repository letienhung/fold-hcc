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
	Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QDesignerCustomWidgetInterface" /* FILE "qledindicator.json" */)
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

	bool showDecomp;
	bool showKeyframe;

	bool showAABB;
	bool showCuboid;
	bool showScaffold;
	bool showMesh;
	bool showAFS;

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
	void setShowDecomp(int state);
	void setShowKeyframe(int state);

	void setShowAABB(int state);
	void setShowCuboid(int state);
	void setShowScaffold(int state);
	void setShowMesh(int state);
	void setShowAFS(int state);

	// scene and message
	void updateScene();
	void resetScene();
	void showStatus(QString msg);

	// export
	void exportCurrent();
	void exportAllObj();
	void exportSVG();
	void exportPNG();

	// color dialog
	void showColorDialog();
	void updateSelNodesColor(QColor c);
	void colorMasterSlave();

	// snapshot
	void saveSnapshot();
	void saveSnapshotAll();

	// hide
	void hideSelectedNodes();
	void unhideAllNodes();
	void hideEdgeRods();

	// debug
	void test1();
	void test2();
};


