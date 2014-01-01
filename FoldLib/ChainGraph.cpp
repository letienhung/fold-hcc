#include "ChainGraph.h"
#include "FdUtility.h"
#include "RodNode.h"

ChainGraph::ChainGraph( FdNode* part, PatchNode* panel1, PatchNode* panel2 /*= NULL*/ )
	: FdGraph(part->mID)
{
	// clone parts
	mParts << (FdNode*)part->clone();
	Structure::Graph::addNode(mParts[0]);

	mPanels << (PatchNode*)panel1->clone();
	mPanels[0]->isCtrlPanel = true;
	Graph::addNode(mPanels[0]);

	if (panel2)
	{
		mPanels << (PatchNode*)panel2->clone();
		mPanels[1]->isCtrlPanel = true;
		Graph::addNode(mPanels[1]);
	}

	// detect hinges
	rootJointSegs = detectHingeSegments(mParts[0], mPanels[0]);

	// upSeg
	Geom::Segment axisSeg = rootJointSegs[0];
	Vector3 origin = axisSeg.P0;
	if (mParts[0]->mType == FdNode::PATCH)
	{
		PatchNode* partPatch = (PatchNode*)mParts[0];
		QVector<Geom::Segment> edges = partPatch->mPatch.getPerpEdges(axisSeg.Direction);
		chainUpSeg = edges[0].contains(origin) ? edges[0] : edges[1];
	}
	else
	{
		RodNode* partRod = (RodNode*)mParts[0];
		chainUpSeg = partRod->mRod;
	}
	if (chainUpSeg.getProjCoordinates(origin) > 0) chainUpSeg.flip();

	// righV
	foreach (Geom::Segment hinge, rootJointSegs)
	{
		Vector3 crossAxisV1 = cross(axisSeg.Direction, chainUpSeg.Direction);
		Vector3 rV = mPanels[0]->mPatch.getProjectedVector(crossAxisV1);

		rootRightVs.push_back(rV.normalized());
	}

	// initial 
	isReady = false;
}

QVector<Structure::Node*> ChainGraph::getKeyframeNodes( double t, bool withPanels )
{
	// apply fold
	fold(t);

	// clone nodes
	QVector<Structure::Node*> knodes;
	if (withPanels)
	{
		foreach(PatchNode* n, mPanels)
			knodes << n->clone();
	}

	foreach(FdNode* n, mParts)
		knodes << n->clone();

	return knodes;
}

