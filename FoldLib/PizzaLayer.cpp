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

	// create chains
	double thr = mPanel->mBox.getExtent(mPanel->mPatch.Normal) * 2;
	foreach (FdNode* n, getFdNodes())
	{
		if (n->isCtrlPanel) continue;

		if (getDistance(n, mPanel) < thr)
		{
			QString cid = QString::number(chains.size());
			chains.push_back(new PizzaChain(n, mPanel, cid));
		}
	}
}

void PizzaLayer::buildDepGraph()
{
	 
}
