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
		FoldingNode* folding = fnSequence[i];
		chain->resolveCollision(folding);
		chain->setupActiveLinks(folding);
	}
}

void LayerGraph::computeChainSequence()
{
	chainSequence.clear();
	fnSequence.clear();
	foreach(DependGraph* dyg, dygSequence) delete dyg;
	dygSequence.clear();

	// set up tags
	foreach(Structure::Node* node, dy_graph->nodes)
	{
		node->properties["visited"] = false;
	}

	// fold chain one by one
	for (int i = 0; i < chains.size(); i++)
	{
		// update scores for each non-visited folding node
		foreach (FoldingNode* fn, dy_graph->getAllFoldingNodes())
		{
			if (fn->properties["visited"].toBool()) continue;

			fn->gain = computeGain(fn->mID);
			fn->cost = computeCost(fn->mID);
		}

		// get best folding node
		FoldingNode* best_fn = dy_graph->getBestFoldingNode();
		ChainNode* best_cn = dy_graph->getChainNode(best_fn->mID);
		chainSequence << best_cn->mID;
		fnSequence << best_fn;

		// save dependency graph sequence
		best_cn->properties["selected"] = true;
		best_fn->properties["selected"] = true;
		dygSequence << (DependGraph*)dy_graph->clone();

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

	// the resulted dependency graph
	dygSequence << (DependGraph*)dy_graph->clone();
}

void LayerGraph::snapshot( double t )
{
	foreach(ChainGraph* chain, chains)
		chain->fold(t);
}

void LayerGraph::outputDyGraphSequence()
{
	for (int i = 0; i < dygSequence.size(); i++)
	{
		QString filePath = path + "/" + mID + "_" + QString::number(i);
		dygSequence[i]->saveAsImage(filePath);
	}
}

double LayerGraph::computeGain( QString fnid )
{
	int gain = 0;

	// clone the graph
	DependGraph* g_copy = (DependGraph*)dy_graph->clone();

	// exclude free chains by themselves
	foreach(ChainNode* cnode, g_copy->getAllChainNodes())
	{
		cnode->properties["visited"] = g_copy->isFreeChainNode(cnode->mID);
	}

	// propagate 
	QQueue<ChainNode*> freeChains;
	ChainNode* cn = g_copy->getChainNode(fnid);
	cn->properties["visited"] = true;
	freeChains.enqueue(cn);

	while(!freeChains.isEmpty())
	{
		ChainNode* cnode = freeChains.dequeue();

		// remove collision links
		foreach (Structure::Link* link, g_copy->getFamilyCollisionLinks(cnode->mID))
		{
			g_copy->removeLink(link);
			gain++;
		}

		// update free chains 
		foreach(ChainNode* cn, g_copy->getAllChainNodes())
		{
			if (!cn->properties["visited"].toBool() && g_copy->isFreeChainNode(cn->mID))
			{
				cn->properties["visited"] = true;
				freeChains.enqueue(cn);
			}
		}
	}

	delete g_copy;

	return gain;
}