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
	/** a stand-alone scaffold representing the folded block at given time
		this scaffold can be requested for position of folded master and slave parts
		this scaffold need be translated to combine with key frame scaffold from other blocks
		to form the final folded scaffold
		this scaffold has to be deleted by whoever calls this function
	**/
	virtual FdGraph* getKeyframeScaffold(double t) = 0;
	

public:
	BLOCK_TYPE mType;
	int selChainIdx;
	QVector<ChainGraph*> chains;
}; 

