#include "PizzaChain.h"
#include "FdUtility.h"
#include "RodNode.h"

PizzaChain::PizzaChain( FdNode* part, PatchNode* panel )
	:FdGraph(part->mID)
{
	// type
	properties["type"] = "pizza";

	// clone parts
	mPart = (FdNode*)part->clone();
	mPanel = (PatchNode*)panel->clone();
	mPanel->isCtrlPanel = true;

	Structure::Graph::addNode(mPart);
	Structure::Graph::addNode(mPanel);

	// detect hinges
	hinges = detectHinges(mPart, mPanel);

	// r1
	Geom::Segment axisSeg = hinges[0];
	Vector3 origin = axisSeg.P0;
	if (mPart->mType == FdNode::PATCH)
	{
		PatchNode* partPatch = (PatchNode*)mPart;
		QVector<Geom::Segment> edges = partPatch->mPatch.getPerpEdges(axisSeg.Direction);
		r1 = edges[0].contains(origin) ? edges[0] : edges[1];
	}
	else
	{
		RodNode* partRod = (RodNode*)mPart;
		r1 = partRod->mRod;
	}
	if (r1.getProjCoordinates(origin) > 0) r1.flip();

	// v2
	foreach (Geom::Segment hinge, hinges)
	{
		Vector3 crossAxisV1 = cross(axisSeg.Direction, r1.Direction);
		Vector3 v2 = mPanel->mPatch.getProjectedVector(crossAxisV1);

		v2s.push_back(v2.normalized());
	}
}

Geom::SectorCylinder PizzaChain::getFoldingVolume( FoldingNode* fn )
{
	Geom::Segment axisSeg = hinges[fn->hingeIdx];
	Vector3 v2 = v2s[fn->hingeIdx];
	if (fn->direct == FD_LEFT) v2 *= -1;
	 
	return Geom::SectorCylinder(axisSeg, r1, v2);
}
