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
	// input
	FdGraph* scaffold; 

	// decomposition
	StrArray2D masterGroups;
	int selDcIdx;
	QVector<DcGraph*> dcGraphs;

public:/// Main pipeline	
	// input
	void setScaffold(FdGraph* fdg);

	// masters
	void identifyMasters();
	void identifyParallelMasters();
	
	// decomposition
	void decompose();

	// foldem
	void foldabilize();

	// output
	void generateKeyframes();
	void exportResultMesh();

public:
	// update Ui
	void updateDcList();
	void updateBlockList();
	void updateChainList();
	void updateKeyframeList();

	// getters
	FdGraph* activeScaffold();
	QStringList getDcGraphLabels();
	DcGraph* getSelDcGraph();
	BlockGraph* getSelBlock();
	FdGraph* getSelKeyframe();

public slots:
	// selection signal from Ui
	void selectDcGraph(QString id);
	void selectBlock(QString id);
	void selectChain(QString id);
	void selectKeyframe(int idx);

	// other signals from Ui
	void foldbzSelBlock();
	void snapshotSelBlock(double t);

signals:
	// notify others about changes
	void sceneChanged();
	void DcGraphsChanged(QStringList labels);
	void blocksChanged(QStringList labels);
	void chainsChanged(QStringList labels);
	void keyframesChanged(int N);
	void resultsGenerated();
};

