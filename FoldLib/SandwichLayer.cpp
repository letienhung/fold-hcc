#include "SandwichLayer.h"
#include "SandwichChain.h"
#include "PatchNode.h"
#include "FdUtility.h"

SandwichLayer::SandwichLayer( QVector<FdNode*> nodes, PatchNode* panel1, PatchNode* panel2, QString id )
	:LayerGraph(nodes, panel1, panel2, id)
{
	mType = LayerGraph::SANDWICH;

	mPanel1 = (PatchNode*) getNode(panel1->mID);
	mPanel2 = (PatchNode*) getNode(panel2->mID);
	mPanel1->isCtrlPanel = true;
	mPanel2->isCtrlPanel = true; 

	// create chains
	double thr = mPanel1->mBox.getExtent(mPanel1->mPatch.Normal) * 2;
	QVector<FdNode*> panels;
	panels << mPanel1 << mPanel2;

	foreach (FdNode* n, getFdNodes())
	{
		if (n->isCtrlPanel) continue;

		if (getDistance(n, panels) < thr)
			chains.push_back(new SandwichChain(n, mPanel1, mPanel2, n->mID));
	}
}

