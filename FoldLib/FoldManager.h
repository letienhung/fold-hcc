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
	Vector3 sqzV;
	FdGraph* dcScaffold;
	StrArray2D masterIdGroups;
	int selDcIdx;
	QVector<DcGraph*> dcGraphs;

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
	/// Main pipeline	
	// input
	void setScaffold(FdGraph* fdg);

	// masters
	void identifyMasters();

	// decomposition
	void setSqzV (QString sqzV_str);
	void decompose();

	// foldem
	void foldabilize();
	void foldbzSelBlock();

	// keyframes
	void generateKeyframes(int nbKeyframes);

	// output
	void exportCollFOG();
	void exportResultMesh();

	// selection signal from Ui
	void selectDcGraph(QString id);
	void selectBlock(QString id);
	void selectChain(QString id);
	void selectKeyframe(int idx);

	// reset dc graphs
	void clearDcGraphs();

signals:
	// notify others about changes
	void sceneChanged();
	void DcGraphsChanged(QStringList labels);
	void blocksChanged(QStringList labels);
	void chainsChanged(QStringList labels);
	void keyframesChanged(int N);
	void message(QString msg);
};

