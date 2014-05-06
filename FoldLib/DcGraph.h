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
	QVector<PatchNode*> masterPatches;

	// slaves
	QVector<FdNode*> slaves;
	QVector< QSet<int> > slave2masterSide; // each slave has one or two end props (master + side).
	QList< QSet<int> > endClusters;
	QVector<int> TSlaves;
	QVector< QSet<int> > HSlaveClusters;// H-slaves sharing side prop belong to the same cluster.

	// blocks
	int selBlockIdx;
	QVector<BlockGraph*> blocks;
	QMap<QString, QSet<int> > masterBlockMap;

	// dependency graph
	FoldOptionGraph* depFog;
	QVector<FoldOptionGraph*> depFogSequence;

	// fold solution
	QVector<BlockGraph*> blockSequence;
	QVector<FoldOption*> foldOptionSequence;

	// time intervals
	double timeScale;
	QVector<TimeInterval> blockTimeIntervals; // the same order as fold solution

	// folding results
	int keyfameIdx;
	QVector<FdGraph*> keyframes;

public:
	// decomposition
	void createMasters(StrArray2D& masterGroups);
	void computeSlaveMasterRelation(); // slave2masterSide
	void createSlaves();
	void clusterSlaves();
	void createBlocks();
	QVector<FdNode*> mergeConnectedCoplanarParts(QVector<FdNode*> ns);

	// foldem
	void foldabilize();
	void buildDepGraph();
	void computeDepLinks();
	void addDepLinkTOptionTEntity(FoldOption* fn, FoldEntity* other_bn);
	void addDepLinkTOptionHEntity(FoldOption* fn, FoldEntity* other_bn);
	void addDepLinkHOptionTEntity(FoldOption* fn, FoldEntity* other_bn);
	void addDepLinkHOptionHEntity(FoldOption* fn, FoldEntity* other_bn);

	FoldOption* getMinCostFreeFoldOption();
	void exportDepFOG();

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

