#pragma once

#include "ChainGraph.h"

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

	// foldabilize
	void foldabilize();

	// fold option graph
	void exportFOG();

	// keyframes
	virtual void snapshot(double t);
	virtual QVector<Structure::Node*> getKeyFrameNodes(double t) = 0;

public:
	BLOCK_TYPE mType;
	int selChainIdx;
	QVector<ChainGraph*> chains;
}; 

