#include "TBlock.h"
#include "TChain.h"
#include "RodNode.h"
#include "PatchNode.h"
#include "FdUtility.h"
#include "DistSegSeg.h"
#include "DistSegRect.h"
#include "Numeric.h"
#include <QDir>

TBlock::TBlock( PatchNode* master, FdNode* slave, QString id )
	:BlockGraph(id)
{
	// type
	mType = BlockGraph::T_BLOCK;

	// clone parts
	Structure::Graph::addNode(slave->clone());
	Structure::Graph::addNode(master->clone());

	// create the chain
	chains << new TChain(master, slave);
}

TBlock::~TBlock()
{
}


QVector<Structure::Node*> TBlock::getKeyFrameParts( double t )
{
	return chains.front()->getKeyframeParts(t);
}


QVector<FoldOption*> TBlock::generateFoldOptions()
{
	return chains.front()->generateFoldOptions();
}

void TBlock::applyFoldOption( FoldOption* fn )
{
	// forward message to the only T-chain
	chains.first()->applyFoldOption(fn);
}

int TBlock::nbTimeUnits()
{
	return 1;
}
