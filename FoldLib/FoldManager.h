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

public:
	// input
	FdGraph* scaffold; 

	// decomposition
	FdGraph* dcScaffold;
	StrArray2D masterIdGroups;
	int selDcIdx;
	QVector<DcGraph*> dcGraphs;

	/// Main pipeline	
public slots:
	// input
	void setScaffold(FdGraph* fdg);

public:
	// masters
	void identifyMasters(QString method, QString direct);
	void identifyParallelMasters(Vector3 squeezeDirect);

public slots:
	// decomposition
	void decompose();

	// foldem
	void foldabilize();
	void exportDepFOG();

public:

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

	// reset dc graphs
	void clearDcGraphs();

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

