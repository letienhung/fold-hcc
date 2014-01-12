#include "SandwichLayer.h"
#include "SandwichChain.h"
#include "PatchNode.h"
#include "FdUtility.h"
#include "IntrRect2Rect2.h"
#include "Numeric.h"

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
			Geom::Rectangle2 fArea1 = chain->getFoldingArea(fn1);
			fn1->properties["fArea"].setValue(fArea1);
			dy_graph->addNode(fn1);

			QString fnid2 = chain->mID + "_" + QString::number(2*j+1);
			FoldingNode* fn2 = new FoldingNode(2*j+1, fnid2);
			Geom::Rectangle2 fArea2 = chain->getFoldingArea(fn2);
			fn2->properties["fArea"].setValue(fArea2);
			dy_graph->addNode(fn2);

			// folding links
			dy_graph->addFoldingLink(cn, fn1);
			dy_graph->addFoldingLink(cn, fn2);

			// debug
			Geom::Rectangle rect1 = mPanel1->mPatch.getRectangle(fArea1);
			Geom::Rectangle rect2 = mPanel1->mPatch.getRectangle(fArea2);
			QVector<Geom::Segment> segs;
			segs += rect1.getEdgeSegments();
			segs += rect2.getEdgeSegments();
			addDebugSegments(segs);
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
		Geom::Rectangle2 fArea = fn->properties["fArea"].value<Geom::Rectangle2>();

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

			// skip siblings
			ChainNode* other_cn = dy_graph->getChainNode(other_fn->mID);
			if (cn == other_cn) continue; 

			Geom::Rectangle2 other_fArea = other_fn->properties["fArea"].value<Geom::Rectangle2>();
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

double SandwichLayer::computeCost( QString fnid )
{
	FoldingNode* fn = (FoldingNode*)dy_graph->getNode(fnid);
	Geom::Rectangle2 fArea = fn->properties["fArea"].value<Geom::Rectangle2>();

	// samples along edges of fArea in 3D
	int nbSamples = 20;
	Geom::Rectangle fArea3D = mPanel1->mPatch.getRectangle(fArea);
	QVector<Vector3> fAreaSamples = fArea3D.getEdgeSamples(nbSamples);

	// faces of barrier box
	QVector<Geom::Plane> barrierPlanes = barrierBox.getFacePlanes();

	// compute the coordinates of colliding points, which are samples from edges
	QVector<Vector2> hotPoints;
	foreach (Structure::Link* link, dy_graph->getCollisionLinks(fnid))
	{
		Structure::Node* other_node = link->getNodeOther(fnid);

		// collision with other folding node
		if (other_node->properties["type"] == "folding")
		{
			FoldingNode* other_fn = (FoldingNode*)other_node;
			Geom::Rectangle2 other_fArea = other_fn->properties["fArea"].value<Geom::Rectangle2>();
			foreach (Vector2 p, other_fArea.getEdgeSamples(nbSamples))
			{
				if (fArea.contains(p)) hotPoints << p;
			}
		}
		// collision with barrier
		else
		{
			BarrierNode* bnode = (BarrierNode*)other_node;
			Geom::Plane bplane = barrierPlanes[bnode->faceIdx];

			// estimate the barrier in 2D using samples from fArea and projection
			foreach(Vector3 sample, fAreaSamples)
			{
				Vector3 bsample = bplane.getProjection(sample);
				Vector2 bsample2D = mPanel1->mPatch.getProjCoordinates(bsample);

				if (fArea.contains(bsample2D)) hotPoints << bsample2D;
			}
		}
	}

	// shrink fArea to avoid all collisions
	Geom::Rectangle2 sfArea = fArea;
	QString cid = dy_graph->getChainNode(fnid)->mID;
	SandwichChain* chain = (SandwichChain*)getChain(cid);
	Geom::Segment2 fAxis2D = chain->getFoldingAxis2D(fn);
	sfArea.shrinkToAvoidPoints(hotPoints, fAxis2D);

	// save sfArea for further use
	fn->properties["sfArea"].setValue(sfArea);

	// cost
	double cost = 1 - sfArea.area() / fArea.area();
	return cost;
}
