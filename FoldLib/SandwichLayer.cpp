#include "SandwichLayer.h"
#include "SandwichChain.h"
#include "PatchNode.h"
#include "FdUtility.h"

SandwichLayer::SandwichLayer( QVector<FdNode*> nodes, FdNode* panel1, FdNode* panel2, QString id )
	:LayerGraph(nodes, panel1, panel2, id)
{
	this->panel1 = (FdNode*) getNode(panel1->id);
	this->panel2 = (FdNode*) getNode(panel2->id);

	// create chains
	PatchNode* pp = (PatchNode*)this->panel1;
	double thr = pp->mBox.getExtent(pp->mPatch.Normal) * 2;
	QVector<FdNode*> panels;
	panels << this->panel1 << this->panel2;

	foreach (FdNode* n, getFdNodes())
	{
		if (n->isCtrlPanel) continue;

		if (getDistance(n, panels) < thr)
		{
			QString cid = QString::number(chains.size());
			chains.push_back(new SandwichChain(n, this->panel1, this->panel2, cid));
		}
	}
}

