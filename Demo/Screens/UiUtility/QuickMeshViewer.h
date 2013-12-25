#pragma once

#include "QGLViewer/qglviewer.h"
using namespace qglviewer;
#define GL_MULTISAMPLE 0x809D

#include "FdGraph.h"

using namespace SurfaceMesh;

class LoaderThread;

class QuickMeshViewer : public QGLViewer{
	Q_OBJECT
public:
	QuickMeshViewer(QWidget * parent = 0);

	virtual void init();
	virtual void resetView();
	virtual void draw();
	virtual void preDraw();
	virtual void postDraw();

	void focusInEvent( QFocusEvent * event );
	void clearGraph();

	bool isActive;
	FdGraph *mGraph;
	bool isLoading;

	QString graphFileName() { return mGraph->path; }

public slots:
	void loadGraph(QString fileName);

signals:
	void gotFocus(QuickMeshViewer*);
	void graphLoaded();
};
