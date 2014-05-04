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


QVector<Structure::Node*> TBlock::getKeyFrameNodes( double t )
{
	QVector<Structure::Node*> knodes;

	// evenly distribute time among pizza chains
	QVector<double> chainStarts = getEvenDivision(chains.size());

	// chain parts
	// fold in sequence
	for (int i = 0; i < chains.size(); i++)
	{
		double lt = getLocalTime(t, chainStarts[i], chainStarts[i+1]);
		knodes += chains[i]->getKeyframeParts(lt);
	}

	// control panels
	if (chains.isEmpty())
	{
		// empty layer: panel is the only part
		knodes += nodes.front()->clone(); 
	}
	else
	{
		// layer with chains: get panels from first chain
		double lt = getLocalTime(t, chainStarts[0], chainStarts[1]);
		knodes += chains.front()->getKeyFramePanels(lt);
	}

	return knodes;
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
