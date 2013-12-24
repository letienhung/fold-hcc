#include "PizzaLayer.h"
#include "PizzaChain.h"
#include "PatchNode.h"
#include "FdUtility.h"


PizzaLayer::PizzaLayer( QVector<FdNode*> nodes, PatchNode* panel, QString id )
	:LayerGraph(nodes, NULL, panel, id)
{
	// type
	mType = LayerGraph::PIZZA;

	// panel
	mPanel = (PatchNode*)getNode(panel->mID);
	mPanel->isCtrlPanel = true;

	// create chains
	double thr = mPanel->mBox.getExtent(mPanel->mPatch.Normal) * 2;
	foreach (FdNode* n, getFdNodes())
	{
		if (n->isCtrlPanel) continue;

		if (getDistance(n, mPanel) < thr)
			chains.push_back(new PizzaChain(n, mPanel, n->mID));
	}

	buildDepGraph();
}


PizzaLayer::~PizzaLayer()
{
	delete dy_graph;
}

void PizzaLayer::buildDepGraph()
{
	dy_graph = new DependGraph(this);

	for(int i = 0; i < chains.size(); i++)
	{
		PizzaChain* chain = (PizzaChain*)chains[i];

		// chain nodes
		ChainNode* cn = new ChainNode(i, chain->mID);
		dy_graph->addNode(cn);

		for (int j = 0; j < chain->hinges.size(); j++)
		{
			// folding nodes
			QString fnid1 = chain->mID + "_" + QString::number(2*j);
			FoldingNode* fn1 = new FoldingNode(j, FD_LEFT, fnid1);
			dy_graph->addNode(fn1);

			QString fnid2 = chain->mID + "_" + QString::number(2*j+1);
			FoldingNode* fn2 = new FoldingNode(j, FD_LEFT, fnid2);
			dy_graph->addNode(fn2);

			// folding links
			dy_graph->addFoldingLink(cn, fn1);
			dy_graph->addFoldingLink(cn, fn2);
		}
	}

	// collision links

}
