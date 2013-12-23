#include "PizzaLayer.h"
#include "PizzaChain.h"
#include "PatchNode.h"
#include "FdUtility.h"

PizzaLayer::PizzaLayer( QVector<FdNode*> nodes, FdNode* panel, QString id )
	:LayerGraph(nodes, NULL, panel, id)
{
	this->panel = (FdNode*) getNode(panel->id);

	// create chains
	PatchNode* pp = (PatchNode*)this->panel;
	double thr = pp->mBox.getExtent(pp->mPatch.Normal) * 2;

	foreach (FdNode* n, getFdNodes())
	{
		if (n->isCtrlPanel) continue;

		if (getDistance(n, pp) < thr)
		{
			QString cid = QString::number(chains.size());
			chains.push_back(new PizzaChain(n, this->panel, cid));
		}
	}
}