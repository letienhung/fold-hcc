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
	foreach(ChainGraph* c, chains)
		delete c;

	delete dy_graph;
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

void LayerGraph::fold()
{
	buildDependGraph();
	computeChainSequence();

	for (int i = 0; i < chainSequence.size(); i++)
	{
		ChainGraph* chain = getChain(chainSequence[i]);
		chain->prepareFolding(fnSequence[i]);
	}
}

void LayerGraph::computeChainSequence()
{
	chainSequence.clear();
	fnSequence.clear();

	// set up tags
	foreach(Structure::Node* node, dy_graph->nodes)
	{
		node->properties["visited"] = false;
	}

	// fold chain one by one
	for (int i = 0; i < chains.size(); i++)
	{
		// update scores for each folding node
		dy_graph->computeScores();

		// output dependency graph
		QString filePath = path + "/" + mID + "_" + QString::number(i);
		dy_graph->saveAsImage(filePath);

		// get best folding node
		FoldingNode* best_fn = dy_graph->getBestFoldingNode();
		ChainNode* best_cn = dy_graph->getChainNode(best_fn->mID);
		chainSequence << best_cn->mID;
		fnSequence << best_fn;

		// exclude family nodes
		foreach(Structure::Node* n, dy_graph->getFamilyNodes(best_fn->mID))
		{
			n->properties["visited"] = true;
		}

		// remove collision links incident to the family
		foreach(Structure::Link* clink, dy_graph->getFamilyCollisionLinks(best_cn->mID))
		{
			dy_graph->removeLink(clink);
		}
	}

	// folding sequence
	qDebug() << "Chain sequence: " << QStringList(chainSequence.toList());
}