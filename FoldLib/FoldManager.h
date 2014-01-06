#pragma once

#include "FdGraph.h"
#include "DcGraph.h"
#include "Numeric.h"
#include <QObject>

class FoldManager : public QObject
{
	Q_OBJECT

public:
    FoldManager();
	~FoldManager();
	void clearDcGraphs();

public:
	FdGraph* scaffold; 
	int pushAxis; // X, Y, Z, ALL

	int selId;
	QVector<DcGraph*> dcGraphs;

	// folding results
	int keyIndx;
	QVector< QVector<FdGraph*> > results;

public:
	void createDcGraphs(Vector3 pushDirect, bool addVirtualPanels); 

	FdGraph* activeScaffold();
	DcGraph* getSelDcGraph();
	QStringList getDcGraphLabels();
	void updateLists();

	LayerGraph* getSelLayer();

// MAIN UI ACCESS
public slots:	
	// 1. set up shape to be folded (once)
	void setScaffold(FdGraph* fdg);
	// 2. after user selected folding direction (once per axis)
	// \aid = 0(X), 1(Y), 2(Z)
	void foldAlongAxis(int aid);
	void createDcGraphs();

	// 3. invoke when a folding percentage is picked (multiple times)
	// \pc \in (0, 1]
	void generateFdKeyFrames();
	void foldAll();

public slots:
	void selectDcGraph(QString id);
	void selectLayer(QString id);
	void selectChain(QString id);
	void selectKeyframe(int idx);

	void fold();
	void foldSelLayer();
	void snapshotSelLayer(double t);

	FdGraph* getKeyframe();

signals:
	void sceneChanged();
	void lyGraphsChanged(QStringList labels);
	void layersChanged(QStringList labels);
	void chainsChanged(QStringList labels);
	void resultsGenerated(int N);
};

