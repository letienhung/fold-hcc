#include "TBlock.h"
#include "TChain.h"
#include "RodNode.h"
#include "PatchNode.h"
#include "FdUtility.h"
#include "DistSegSeg.h"
#include "DistSegRect.h"
#include "Numeric.h"
#include <QDir>

TBlock::TBlock( PatchNode* master, FdNode* slave, Geom::Box bb, QString id )
	:BlockGraph(id, bb)
{
	// type
	mType = BlockGraph::T_BLOCK;

	// clone parts
	Structure::Graph::addNode(slave->clone());
	Structure::Graph::addNode(master->clone());

	// create the chain
	chains << new TChain(master, slave);

	// base master
	baseMasterId = master->mID;
}

TBlock::~TBlock()
{
}


QVector<FoldOption*> TBlock::generateFoldOptions()
{
	QVector<FoldOption*> options;
	foreach (FoldOption* fn, chains.front()->generateFoldOptions())
	{
		bool reject = false;
		if (withinAABB)
		{
			// barrier box should contain the folding volume
			Geom::SectorCylinder fV = fn->properties["fVolume"].value<Geom::SectorCylinder>();
			if (!barrierBox.containsAll(fV.getConners()))
				reject = true; 
		}

		if (!reject) options << fn;
	}

	return options;
}

void TBlock::applyFoldOption( FoldOption* fn )
{
	// forward message to the only T-chain
	chains.first()->applyFoldOption(fn);
}

double TBlock::getTimeLength()
{
	return 1.0;
}

FdGraph* TBlock::getKeyframeScaffold( double t )
{
	return chains.front()->getKeyframeScaffold(t);
}
