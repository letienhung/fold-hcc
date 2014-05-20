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
    DcGraph(QString id, FdGraph* scaffold, StrArray2D masterGroups, Vector3 v);
	~DcGraph();

public:
	// masters
	Vector3 sqzV;
	PatchNode* baseMaster;
	QVector<PatchNode*> masters;
	QMultiMap<QString, QString> masterOrderGreater; // key > value
	QMultiMap<QString, QString> masterOrderLess; // key < value

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
	int keyfameIdx;
	QVector<FdGraph*> keyframes;

public:
	// decomposition
	void createMasters(StrArray2D& masterGroups);
	void computeMasterOrderConstraints();

	void createSlaves(); 
	void updateSlaves(); // collect current slaves and store into \p slaves
	void updateSlaveMasterRelation();
	QVector<FdNode*> mergeConnectedCoplanarParts(QVector<FdNode*> ns);

	void createBlocks();
	void clusterSlaves();

	// foldem
	void foldabilize();
	void foldbzSelBlock();
	void findFoldOrderGreedy();
	int getBestNextBlockIndex(double currT);
	bool isValid(FdGraph* folded);

	// export
	void exportCollFOG();

	// key frame
	void generateKeyframes(int N);
	FdGraph* getKeyframe(double t);

public:
	FdGraph* activeScaffold();
	BlockGraph* getSelBlock();

	QStringList getBlockLabels();
	void selectBlock(QString id);

	FdGraph* getSelKeyframe();
};

