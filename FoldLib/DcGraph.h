#pragma once

#include "FdGraph.h"
#include "PatchNode.h"
#include "Numeric.h"
#include "BlockGraph.h"

// DcGraph encodes the decomposition of scaffold
// including base patches and layers

class DcGraph : public FdGraph
{
public:
    DcGraph(FdGraph* scaffold, StrArray2D masterGroups, QString id);
	~DcGraph();

public:
	QVector<PatchNode*> masterPatches;

	int selBlockIdx;
	QVector<BlockGraph*> blocks;

	// folding results
	int keyfameIdx;
	QVector<FdGraph*> keyframes;

public:
	// blocks
	void createBlocks();
	QVector<FdNode*> mergeCoplanarParts(QVector<FdNode*> ns, PatchNode* panel);

	// fold
	void foldabilize();

	// key frame
	FdGraph* getKeyFrame(double t);

public:
	FdGraph* activeScaffold();
	BlockGraph* getSelLayer();

	QStringList getLayerLabels();
	void selectLayer(QString id);
};

