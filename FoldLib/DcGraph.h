#pragma once

#include "FdGraph.h"
#include "PatchNode.h"
#include "Numeric.h"
#include "BlockGraph.h"
#include "FoldOptionGraph.h"
#include "ShapeSuperKeyframe.h"

// DcGraph encodes the decomposition of scaffold
// including base patches and layers

class DcGraph : public FdGraph
{
public:
    DcGraph(QString id, FdGraph* scaffold, Vector3 v, double connThr);
	~DcGraph();

public:
	// squeezing direction
	Vector3 sqzV;

	// masters
	PatchNode* baseMaster; // non-virtual master
	QVector<PatchNode*> masters;
	QMap<QString, QSet<QString> > masterOrderGreater;
	QMap<QString, QSet<QString> > masterOrderLess;

	// slaves
	QVector<FdNode*> slaves;
	QVector< QSet<int> > slave2master;
	QVector< QSet<int> > slaveClusters;

	// blocks
	int selBlockIdx;
	QVector<BlockGraph*> blocks;
	QVector<double> blockWeights;
	QMap<QString, QSet<int> > masterBlockMap;

	// time scale
	double timeScale; // timeScale * block.timeUnits = normalized time

	// folding results
	int keyframeIdx;
	QVector<FdGraph*> keyframes;

	// threshold
	double connThrRatio;

public:
	// threshold
	double getConnectivityThr();

	// decomposition
	FdNodeArray2D getPerpConnGroups();
	void computeMasterOrderConstraints();
	void createMasters();

	void updateSlaves();
	void updateSlaveMasterRelation();
	void createSlaves(); 

	void clusterSlaves();
	BlockGraph* createBlock(QSet<int> sCluster);
	void createBlocks();

	void computeBlockWeights();

	BlockGraph* mergeBlocks( QVector<BlockGraph*> blocks );

	// foldabilization
	void foldabilize();
	double foldabilizeBlock(int bid, double currTime, ShapeSuperKeyframe* currKf, 
									double& nextTime, ShapeSuperKeyframe*& nextKf);
	int getBestNextBlockIndex(double currT, ShapeSuperKeyframe* currKeyframe);

	// key frame
	void generateKeyframes(int N);
	FdGraph* getKeyframe(double t);
	ShapeSuperKeyframe* getShapeSuperKeyframe(double t);

public:
	FdGraph* activeScaffold();
	BlockGraph* getSelBlock();

	QStringList getBlockLabels();
	void selectBlock(QString id);

	FdGraph* getSelKeyframe();
	void selectKeyframe(int idx);
};

