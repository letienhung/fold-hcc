#include "LayerGraph.h"

LayerGraph::LayerGraph( QVector<FdNode*> nodes, PatchNode* panel1, PatchNode* panel2, QString id)
	:FdGraph(id)
{
	// clone nodes
	foreach (FdNode* n, nodes)
	{
		Structure::Graph::addNode(n->clone());
	}

	if (panel1)
	{
		FdNode* cloneP1 = (FdNode*)panel1->clone();
		cloneP1->isCtrlPanel = true;
		Structure::Graph::addNode(cloneP1);
	}

	if (panel2)
	{
		FdNode* cloneP2 = (FdNode*)panel2->clone();
		cloneP2->isCtrlPanel = true;
		Structure::Graph::addNode(cloneP2);
	}

	// selected chain
	selId = -1;

	// dependency graph
	dy_graph = new DependGraph(mID);
}

LayerGraph::~LayerGraph()
{
	foreach(FdGraph* c, chains)
		delete c;

	delete dy_graph;
}

FdGraph* LayerGraph::activeScaffold()
{
	FdGraph* selChain = getSelChain();
	if (selChain)  return selChain;
	else		   return this;
}

FdGraph* LayerGraph::getSelChain()
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
