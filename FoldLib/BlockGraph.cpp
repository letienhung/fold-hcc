#include "BlockGraph.h"
#include "FdUtility.h"

BlockGraph::BlockGraph( QString id, Geom::Box bb )
	: FdGraph(id)
{
	// selected chain
	selChainIdx = -1;

	// aabb
	barrierBox = bb;
}

BlockGraph::~BlockGraph()
{
	foreach(ChainGraph* c, chains)
		delete c;
}

FdGraph* BlockGraph::activeScaffold()
{
	FdGraph* selChain = getSelChain();
	if (selChain)  return selChain;
	else		   return this;
}

ChainGraph* BlockGraph::getSelChain()
{
	if (selChainIdx >= 0 && selChainIdx < chains.size())
		return chains[selChainIdx];
	else
		return NULL;
}

void BlockGraph::selectChain( QString id )
{
	selChainIdx = -1;
	for (int i = 0; i < chains.size(); i++)
	{
		if (chains[i]->mID == id)
		{
			selChainIdx = i;
			break;
		}
	}
}

QStringList BlockGraph::getChainLabels()
{
	QStringList labels;
	foreach(FdGraph* c, chains)
		labels.push_back(c->mID);

	// append string to select none
	labels << "--none--";

	return labels;
}

ChainGraph* BlockGraph::getChain( QString cid )
{
	foreach(ChainGraph* c, chains)
		if(c->mID == cid) return c;

	return NULL;
}

PatchNode* BlockGraph::getBaseMaster()
{
	return (PatchNode*)getNode(baseMasterId);
}
