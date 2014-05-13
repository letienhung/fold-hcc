#pragma once

#include "FdGraph.h"
#include "PatchNode.h"
#include "Numeric.h"
#include "BlockGraph.h"
#include "FoldOptionGraph.h"

// DcGraph encodes the decomposition of scaffold
// including base patches and layers

class DcGraph : public FdGraph
{
public:
    DcGraph(FdGraph* scaffold, StrArray2D masterGroups, QString id);
	~DcGraph();

public:
	// masters
	QString baseMasterId;
	QVector<PatchNode*> masters;

	// slaves
	QVector<FdNode*> slaves;
	QVector< QSet<int> > slave2master;
	QVector< QSet<int> > slave2masterSide; // each slave has one or two end props (master + side).

	QVector<int> TSlaves;
	QVector< QSet<int> > HSlaveClusters;// H-slaves sharing side prop belong to the same cluster.

	// blocks
	int selBlockIdx;
	QVector<BlockGraph*> blocks;
	QMap<QString, QSet<int> > masterBlockMap;

	// fold order
	QVector<BlockGraph*> blockSequence;

	// time scale
	double timeScale; // timeScale * block.timeUnits = normalized time

	// folding results
	int keyfameIdx;
	QVector<FdGraph*> keyframes;

public:
	// decomposition
	void createMasters(StrArray2D& masterGroups);

	void createSlaves(); 
	void updateSlaves(); // collect current slaves and store into \p slaves
	void updateSlaveMasterRelation();
	QVector<FdNode*> mergeConnectedCoplanarParts(QVector<FdNode*> ns);

	void createBlocks();
	void clusterSlaves();

	// foldem
	void foldabilize(bool withinAABB);
	void findFoldOrderGreedy();
	FoldOption* getMinCostFreeFoldOption();

	// export
	void exportCollFOG();

	// key frame
	void generateKeyframes(int N);
	FdGraph* getKeyFrame(double t);

public:
	FdGraph* activeScaffold();
	BlockGraph* getSelBlock();

	QStringList getBlockLabels();
	void selectBlock(QString id);

	FdGraph* getSelKeyframe();
};

