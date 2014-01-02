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
	~QuickMeshViewer();

	virtual void init();
	virtual void resetView();
	virtual void draw();
	virtual void preDraw();
	virtual void postDraw();

	void focusInEvent( QFocusEvent * event );
	void clearGraph();
	void setGraph(FdGraph *graph);

	bool isActive;
	FdGraph *mGraph;
	bool isLoading;
	int mIdx;

	QString graphFileName() { return mGraph->path; }

public slots:
	void loadGraph(QString fileName);

signals:
	void gotFocus(QuickMeshViewer*);
	void graphLoaded();
};
