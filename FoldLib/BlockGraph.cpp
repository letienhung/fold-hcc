#include "BlockGraph.h"
#include "FdUtility.h"

BlockGraph::BlockGraph( QVector<FdNode*> parts, QString id)
	:FdGraph(id)
{
	// clone nodes
	foreach (FdNode* n, parts)
	{
		Structure::Graph::addNode(n->clone());
	}

	// selected chain
	selChainIdx = -1;
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

	return labels;
}

ChainGraph* BlockGraph::getChain( QString cid )
{
	foreach(ChainGraph* c, chains)
		if(c->mID == cid) return c;

	return NULL;
}

void BlockGraph::snapshot( double t )
{
	foreach(ChainGraph* chain, chains)
		chain->fold(t);
}

void BlockGraph::exportFOG()
{
	QString filePath = path + "/" + mID;
	fog->saveAsImage(filePath);
}