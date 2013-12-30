#include "ChainGraph.h"
#include "FdUtility.h"
#include "RodNode.h"

ChainGraph::ChainGraph( FdNode* part, PatchNode* panel1, PatchNode* panel2 /*= NULL*/ )
	: FdGraph(part->mID)
{
	// clone parts
	mPart = (FdNode*)part->clone();
	Structure::Graph::addNode(mPart);

	mPanel1 = (PatchNode*)panel1->clone();
	mPanel1->isCtrlPanel = true;
	Graph::addNode(mPanel1);

	if (panel2)
	{
		mPanel2 = (PatchNode*)panel2->clone();
		mPanel2->isCtrlPanel = true;
		Graph::addNode(mPanel2);
	}

	// detect hinges
	hingeSegs = detectHingeSegments(mPart, mPanel1);

	// upSeg
	Geom::Segment axisSeg = hingeSegs[0];
	Vector3 origin = axisSeg.P0;
	if (mPart->mType == FdNode::PATCH)
	{
		PatchNode* partPatch = (PatchNode*)mPart;
		QVector<Geom::Segment> edges = partPatch->mPatch.getPerpEdges(axisSeg.Direction);
		upSeg = edges[0].contains(origin) ? edges[0] : edges[1];
	}
	else
	{
		RodNode* partRod = (RodNode*)mPart;
		upSeg = partRod->mRod;
	}
	if (upSeg.getProjCoordinates(origin) > 0) upSeg.flip();

	// righV
	foreach (Geom::Segment hinge, hingeSegs)
	{
		Vector3 crossAxisV1 = cross(axisSeg.Direction, upSeg.Direction);
		Vector3 rV = mPanel1->mPatch.getProjectedVector(crossAxisV1);

		rightVs.push_back(rV.normalized());
	}
}

