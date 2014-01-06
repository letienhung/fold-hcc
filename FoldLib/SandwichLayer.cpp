#include "SandwichLayer.h"
#include "SandwichChain.h"
#include "PatchNode.h"
#include "FdUtility.h"
#include "IntrRect2Rect2.h"

SandwichLayer::SandwichLayer( QVector<FdNode*> parts, PatchNode* panel1, PatchNode* panel2, QString id, Geom::Box &bBox )
	:LayerGraph(parts, panel1, panel2, id, bBox)
{
	mType = LayerGraph::SANDWICH;

	mPanel1 = (PatchNode*) getNode(panel1->mID);
	mPanel2 = (PatchNode*) getNode(panel2->mID);
	mPanel1->properties["isCtrlPanel"] = true;
	mPanel2->properties["isCtrlPanel"] = true;

	// create chains
	double thr = mPanel1->mBox.getExtent(mPanel1->mPatch.Normal) * 2;
	QVector<FdNode*> panels;
	panels << mPanel1 << mPanel2;

	foreach (FdNode* n, getFdNodes())
	{
		if (n->properties.contains("isCtrlPanel")) continue;

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

		for (int j = 0; j < chain->rootJointSegs.size(); j++)
		{
			// folding nodes
			QString fnid1 = chain->mID + "_" + QString::number(2*j);
			FoldingNode* fn1 = new FoldingNode(2*j, fnid1);
			dy_graph->addNode(fn1);

			QString fnid2 = chain->mID + "_" + QString::number(2*j+1);
			FoldingNode* fn2 = new FoldingNode(2*j+1, fnid2);
			dy_graph->addNode(fn2);

			// folding links
			dy_graph->addFoldingLink(cn, fn1);
			dy_graph->addFoldingLink(cn, fn2);
		}
	}

	// barrier nodes
	QVector<Geom::Rectangle> barriers = barrierBox.getFaceRectangles();
	Vector3 pNormal = mPanel1->mPatch.Normal;
	for (int i = 0; i < Geom::Box::NB_FACES; i++)
	{
		if (fabs(dot(pNormal, barriers[i].Normal)) > 0.5)
			continue;
		dy_graph->addNode(new BarrierNode(i));
	}

	// collision links
	QVector<FoldingNode*> fns = dy_graph->getAllFoldingNodes();
	for (int i = 0; i < fns.size(); i++)
	{
		FoldingNode* fn = fns[i];
		ChainNode* cn = dy_graph->getChainNode(fn->mID);
		SandwichChain* chain = (SandwichChain*) getChain(cn->mID);
		Geom::Rectangle2 fArea = chain->getFoldingArea(fn);

		// with barriers
		QVector<Vector3> fConners = mPanel1->mPatch.getRectangle(fArea).getConners();
		foreach (BarrierNode* bn, dy_graph->getAllBarrierNodes())
		{
			Geom::Plane bplane = barriers[bn->faceIdx].getPlane();
			if (!bplane.onSameSide(fConners))
			{
				dy_graph->addCollisionLink(fn, bn);
			}
		}

		// with other folding nodes
		for (int j = i+1; j < fns.size(); j++)
		{
			FoldingNode* other_fn = fns[j];
			ChainNode* other_cn = dy_graph->getChainNode(other_fn->mID);

			// skip siblings
			if (cn == other_cn) continue; 

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


QVector<Structure::Node*> SandwichLayer::getKeyFrameNodes( double t )
{
	QVector<Structure::Node*> knodes;

	// chain parts
	// fold all chains simultaneously
	for (int i = 0; i < chains.size(); i++)
		knodes += chains[i]->getKeyframeParts(t);

	// control panels
	if (chains.isEmpty())
		// empty layer: panel is the only part
		knodes += nodes.front()->clone(); 
	else
		// layer with chains: get panels from first chain
		knodes += chains.front()->getKeyFramePanels(t);

	return knodes;
}