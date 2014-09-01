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
	PatchNode* baseMaster;
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
	QMap<QString, QSet<int> > masterBlockMap;

	// fold order
	QVector<BlockGraph*> blockSequence;

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
	void createMasters();
	FdNodeArray2D getPerpConnGroups();
	void computeMasterOrderConstraints();

	void createSlaves(); 
	void updateSlaves();
	void updateSlaveMasterRelation();

	void createBlocks();
	void clusterSlaves();
	BlockGraph* createBlock(QSet<int> sCluster);
	BlockGraph* mergeBlocks( QVector<BlockGraph*> blocks );


	// foldem
	void foldabilize();
	void foldbzSelBlock();
	int getBestNextBlockIndex(double currT, ShapeSuperKeyframe* currKeyframe);

	// export
	void exportCollFOG();

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

