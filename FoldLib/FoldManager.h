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

	// keyframes
	int nbKeyframes;

public:
	// update Ui
	void updateDcList();
	void updateBlockList();
	void updateChainList();
	void updateKeyframeSlider();
	void updateSolutionList();

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

	// parameters
	void setSqzV (QString sqzV_str);
	void setNbSplits(int N);
	void setNbChunks(int N);
	void useThickness(int state);
	void setThinkness(double thk);

	// fold
	void foldabilize();
	void foldbzSelBlock();

	// keyframes
	void setNbKeyframes(int N);
	void generateKeyframes();

	// decomposition
	void identifyMasters();
	void decompose();

	// output
	void exportCollFOG();
	void exportResultMesh();

	// selection signal from Ui
	void selectDcGraph(QString id);
	void selectBlock(QString id);
	void selectChain(QString id);
	void selectKeyframe(int idx);
	void selectSolution(int idx);

	// reset dc graphs
	void clearDcGraphs();

signals:
	// notify others about changes
	void sceneChanged();
	void DcGraphsChanged(QStringList labels);
	void blocksChanged(QStringList labels);
	void chainsChanged(QStringList labels);
	void keyframesChanged(int N);
	void solutionsChanged(int N);
	void message(QString msg);
};

