#include "SandwichLayer.h"
#include "SandwichChain.h"
#include "PatchNode.h"
#include "FdUtility.h"
#include "IntrRect2Rect2.h"

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
		{
			chains.push_back(new SandwichChain(n, mPanel1, mPanel2));
		}
	}
}

void SandwichLayer::buildDependGraph()
{
	// clear
	dy_graph->clear();

	// nodes and folding links
	for(int i = 0; i < chains.size(); i++)
	{
		SandwichChain* chain = (SandwichChain*)chains[i];

		// chain nodes
		ChainNode* cn = new ChainNode(i, chain->mID);
		dy_graph->addNode(cn);

		for (int j = 0; j < chain->hingeSegs.size(); j++)
		{
			// folding nodes
			QString fnid1 = chain->mID + "_" + QString::number(2*j);
			FoldingNode* fn1 = new FoldingNode(j, FD_RIGHT, fnid1);
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
	// between two folding nodes
	QVector<FoldingNode*> fns = dy_graph->getAllFoldingNodes();
	for (int i = 0; i < fns.size(); i++)
	{
		FoldingNode* fn = fns[i];
		ChainNode* cn = dy_graph->getChainNode(fn->mID);
		SandwichChain* chain = (SandwichChain*) getChain(cn->mID);
		Geom::Rectangle2 fArea = chain->getFoldingArea(fn);

		for (int j = i+1; j < fns.size(); j++)
		{
			FoldingNode* other_fn = fns[j];

			ChainNode* other_cn = dy_graph->getChainNode(other_fn->mID);
			if (cn == other_cn) continue; // skip siblings

			SandwichChain* other_chain = (SandwichChain*) getChain(other_cn->mID);
			Geom::Rectangle2 other_fArea = other_chain->getFoldingArea(other_fn);

			// add collision link
			if (Geom::IntrRect2Rect2::test(fArea, other_fArea))
			{
				dy_graph->addCollisionLink(fn, other_fn);
			}
		}
	}
}
