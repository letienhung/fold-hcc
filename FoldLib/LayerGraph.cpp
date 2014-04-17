#include "LayerGraph.h"
#include "FdUtility.h"

LayerGraph::LayerGraph( QVector<FdNode*> parts, PatchNode* panel1, PatchNode* panel2, QString id, Geom::Box &bBox)
	:FdGraph(id), barrierBox(bBox)
{
	// clone nodes
	foreach (FdNode* n, parts)
	{
		Structure::Graph::addNode(n->clone());
	}

	if (panel1)
	{
		PatchNode* cloneP1 = (PatchNode*)panel1->clone();
		cloneP1->properties["isCtrlPanel"] = true;
		Structure::Graph::addNode(cloneP1);
	}

	if (panel2)
	{
		FdNode* cloneP2 = (FdNode*)panel2->clone();
		cloneP2->properties["isCtrlPanel"] = true;
		Structure::Graph::addNode(cloneP2);
	}

	// selected chain
	selId = -1;

	// dependency graph
	fog = new FoldOptionGraph(mID);
}

LayerGraph::~LayerGraph()
{
	foreach(ChainGraph* c, chains)
		delete c;

	delete fog;
}

FdGraph* LayerGraph::activeScaffold()
{
	FdGraph* selChain = getSelChain();
	if (selChain)  return selChain;
	else		   return this;
}

ChainGraph* LayerGraph::getSelChain()
{
	if (selId >= 0 && selId < chains.size())
		return chains[selId];
	else
		return NULL;
}

void LayerGraph::selectChain( QString id )
{
	selId = -1;
	for (int i = 0; i < chains.size(); i++)
	{
		if (chains[i]->mID == id)
		{
			selId = i;
			break;
		}
	}
}

QStringList LayerGraph::getChainLabels()
{
	QStringList labels;
	foreach(FdGraph* c, chains)
		labels.push_back(c->mID);

	return labels;
}

ChainGraph* LayerGraph::getChain( QString cid )
{
	foreach(ChainGraph* c, chains)
		if(c->mID == cid) return c;

	return NULL;
}

void LayerGraph::snapshot( double t )
{
	foreach(ChainGraph* chain, chains)
		chain->fold(t);
}

void LayerGraph::exportFOG()
{
	QString filePath = path + "/" + mID;
	fog->saveAsImage(filePath);
}