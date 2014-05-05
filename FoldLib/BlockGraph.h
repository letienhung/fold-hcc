#pragma once

#include "ChainGraph.h"
#include "FoldOptionGraph.h"

class BlockGraph : public FdGraph
{
public:
	enum BLOCK_TYPE{T_BLOCK, H_BLOCK};

public:
    BlockGraph(QString id);
	~BlockGraph();

	// selection
	ChainGraph* getSelChain();
	FdGraph* activeScaffold();
	void selectChain(QString id);
	QStringList getChainLabels();

	// getter
	ChainGraph* getChain(QString cid);
	virtual int nbTimeUnits() = 0;

	// foldem
	virtual QVector<FoldOption*> generateFoldOptions() = 0;
	virtual void applyFoldOption(FoldOption* fn) = 0;

	// keyframes
	virtual QVector<Structure::Node*> getKeyFrameParts(double t) = 0;

public:
	BLOCK_TYPE mType;
	int selChainIdx;
	QVector<ChainGraph*> chains;
}; 

