#include "PizzaLayer.h"
#include "PizzaChain.h"
#include "RodNode.h"
#include "PatchNode.h"
#include "FdUtility.h"
#include "DistSegSeg.h"
#include "DistSegRect.h"
#include <QDir>


PizzaLayer::PizzaLayer( QVector<FdNode*> parts, PatchNode* panel, QString id, Geom::Box &bBox )
	:LayerGraph(parts, NULL, panel, id, bBox)
{
	// type
	mType = LayerGraph::PIZZA;

	// panel
	mPanel = (PatchNode*)getNode(panel->mID);
	mPanel->properties["isCtrlPanel"] = true;

	// create chains
	double thr = mPanel->mBox.getExtent(mPanel->mPatch.Normal) * 2;
	foreach (FdNode* n, getFdNodes())
	{
		if (n->properties.contains("isCtrlPanel")) continue;

		if (getDistance(n, mPanel) < thr)
		{
			chains.push_back(new PizzaChain(n, mPanel));
		}
	}
}

PizzaLayer::~PizzaLayer()
{
	delete dy_graph;
}

void PizzaLayer::buildDependGraph()
{
	// clear
	dy_graph->clear();

	// empty pizzaLayer
	if (nodes.size() == 1) return;

	// chain nodes and folding links
	for(int i = 0; i < chains.size(); i++)
	{
		PizzaChain* chain = (PizzaChain*)chains[i];

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
	Vector3 pNormal = mPanel->mPatch.Normal;
	for (int i = 0; i < Geom::Box::NB_FACES; i++)
	{
		double dotProd = fabs(dot(pNormal, barriers[i].Normal));
		if (dotProd > 0.5)  continue;
		dy_graph->addNode(new BarrierNode(i));
	}

	// collision links
	foreach (FoldingNode* fn, dy_graph->getAllFoldingNodes())
	{
		ChainNode* cn = dy_graph->getChainNode(fn->mID);
		PizzaChain* chain = (PizzaChain*) getChain(cn->mID);
		Geom::SectorCylinder fVolume = chain->getFoldingVolume(fn);

		// with barriers 
		foreach (BarrierNode* bn, dy_graph->getAllBarrierNodes())
		{
			Geom::Rectangle& barrier = barriers[bn->faceIdx];
			if (fVolume.intersects(barrier))
			{
				dy_graph->addCollisionLink(fn, bn);
			}
		}

		// with other chain nodes
		foreach(ChainNode* other_cn, dy_graph->getAllChainNodes())
		{
			if (cn == other_cn) continue;

			PizzaChain* other_chain = (PizzaChain*) getChain(other_cn->mID);
			FdNode* other_part = other_chain->mParts[0];

			bool collide = false;
			if (other_part->mType == FdNode::PATCH)
			{
				PatchNode* other_patch = (PatchNode*) other_part;
				if (fVolume.intersects(other_patch->mPatch))
				{
					collide = true;
					//debugSegs << Geom::Segment(fVCenter, other_patch->mPatch.Center);
				}
			}else
			{
				RodNode* other_rod = (RodNode*) other_part;
				if (fVolume.intersects(other_rod->mRod))
				{
					collide = true;
					//debugSegs << Geom::Segment(fVCenter, other_rod->mRod.Center);
				}
			}

			// add collision link
			if (collide)
			{
				dy_graph->addCollisionLink(fn, other_cn);
			}
		}
	}
}

QVector<Structure::Node*> PizzaLayer::getKeyFrameNodes( double t )
{
	QVector<Structure::Node*> knodes;

	// evenly distribute time among pizza chains
	QVector<double> chainStarts = getEvenDivision(chains.size());

	// chain parts
	// fold in sequence
	for (int i = 0; i < chains.size(); i++)
	{
		double lt = getLocalTime(t, chainStarts[i], chainStarts[i+1]);
		ChainGraph* chain = getChain(chainSequence[i]);
		knodes += chain->getKeyframeParts(lt);
	}

	// control panels
	if (chains.isEmpty())
	{
		// empty layer: panel is the only part
		knodes += nodes.front()->clone(); 
	}
	else
	{
		// layer with chains: get panels from first chain
		double lt = getLocalTime(t, chainStarts[0], chainStarts[1]);
		knodes += chains.front()->getKeyFramePanels(lt);
	}

	return knodes;
}

void PizzaLayer::resolveCollision()
{

}

Vector3 PizzaLayer::getClosestCoordinates(Geom::SectorCylinder& fVolume, FdNode* node)
{
	Geom::Segment axisSeg = fVolume.getAxisSegment();
	Vector3 closestP;
	if (node->mType == FdNode::PATCH)
	{
		PatchNode* pnode = (PatchNode*) node;
		Geom::DistSegRect dsr(axisSeg, pnode->mPatch);
		closestP = dsr.mClosestPoint1;
	}
	else
	{
		RodNode* rnode = (RodNode*) node;
		Geom::DistSegSeg dss(axisSeg, rnode->mRod);
		closestP = dss.mClosestPoint1;
	}

	//addDebugSegment(Geom::Segment(fVolume.getAxisSegment().Center, closestP));

	return fVolume.getCoordinates(closestP);
}

Vector3 PizzaLayer::getClosestCoordinates( Geom::SectorCylinder& fVolume, Geom::Rectangle& rect )
{
	Geom::Segment axisSeg = fVolume.getAxisSegment();
	Geom::DistSegRect dsr(axisSeg, rect);

	Vector3 closestP = dsr.mClosestPoint1;
	//addDebugSegment(Geom::Segment(fVolume.getAxisSegment().Center, closestP));

	return fVolume.getCoordinates(closestP);
}

double PizzaLayer::computeCost( QString fnid )
{
	FoldingNode* fn = (FoldingNode*)dy_graph->getNode(fnid);
	QString cid = dy_graph->getChainNode(fnid)->mID;
	PizzaChain* chain = (PizzaChain*)getChain(cid);
	Geom::SectorCylinder fVolume = chain->getFoldingVolume(fn);
	
	// compute the coordinates of closest colliding point in fVolume
	QVector<Vector3> hotCoords;
	foreach (Structure::Link* link, dy_graph->getCollisionLinks(fnid))
	{
		Structure::Node* other_node = link->getNodeOther(fnid);

		// collision with other chain
		if (other_node->properties["type"] == "chain")
		{
			PizzaChain* other_chain = (PizzaChain*)getChain(other_node->mID);
			FdNode* other_part = other_chain->mParts[0];
			hotCoords << getClosestCoordinates(fVolume, other_part);

		}
		// collision with barrier
		else
		{
			BarrierNode* bnode = (BarrierNode*)other_node;
			QVector<Geom::Rectangle> bfaces = barrierBox.getFaceRectangles();
			Geom::Rectangle brect = bfaces[bnode->faceIdx];
			hotCoords << getClosestCoordinates(fVolume, brect);
		}

		//addDebugSegment(Geom::Segment(fVolume.getAxisSegment().Center, fVolume.getPosition(hotCoords.last())));
	}

	// shrink fVolume to avoid all collisions
	double minRadius = 1;
	foreach(Vector3 coord, hotCoords)
	{ 
		if (coord.x() < minRadius) 
			minRadius = coord.x();
	}

	// the cost is the volume lost of fVolume
	double cost = 1 - minRadius;

	// save the shrunk fVolume for further use
	fVolume.Radius *= minRadius;
	fn->properties["fVolume"].setValue(fVolume);

	return cost;
}
