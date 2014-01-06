#include "ChainGraph.h"
#include "FdUtility.h"
#include "RodNode.h"

ChainGraph::ChainGraph( FdNode* part, PatchNode* panel1, PatchNode* panel2)
	: FdGraph(part->mID)
{
	// clone parts
	mParts << (FdNode*)part->clone();
	Structure::Graph::addNode(mParts[0]);

	mPanels << (PatchNode*)panel1->clone();
	mPanels[0]->properties["isCtrlPanel"] = true;
	Graph::addNode(mPanels[0]);

	if (panel2)
	{
		mPanels << (PatchNode*)panel2->clone();
		mPanels[1]->properties["isCtrlPanel"] = true;
		Graph::addNode(mPanels[1]);
	}

	// detect hinges
	rootJointSegs = detectJointSegments(mParts[0], mPanels[0]);

	// upSeg
	Geom::Segment jointSeg = rootJointSegs[0];
	Vector3 origin = jointSeg.P0;
	if (mParts[0]->mType == FdNode::PATCH)
	{
		PatchNode* partPatch = (PatchNode*)mParts[0];
		QVector<Geom::Segment> edges = partPatch->mPatch.getPerpEdges(jointSeg.Direction);
		chainUpSeg = edges[0].contains(origin) ? edges[0] : edges[1];
	}
	else
	{
		RodNode* partRod = (RodNode*)mParts[0];
		chainUpSeg = partRod->mRod;
	}
	if (chainUpSeg.getProjCoordinates(origin) > 0) chainUpSeg.flip();

	// righV
	foreach (Geom::Segment rjs, rootJointSegs)
	{
		Vector3 crossAxisV1 = cross(rjs.Direction, chainUpSeg.Direction);
		Vector3 rV = mPanels[0]->mPatch.getProjectedVector(crossAxisV1);

		rootRightVs.push_back(rV.normalized());
	}

	// initial 
	isReady = false;
}

QVector<Structure::Node*> ChainGraph::getKeyframeParts( double t )
{
	// apply fold
	fold(t);

	// clone nodes
	QVector<Structure::Node*> knodes;
	foreach(FdNode* n, mParts)
		knodes << n->clone();

	return knodes;
}

QVector<Structure::Node*> ChainGraph::getKeyFramePanels( double t )
{
	// apply fold
	fold(t);

	// clone nodes
	QVector<Structure::Node*> knodes;
	foreach(FdNode* n, mPanels)
		knodes << n->clone();

	return knodes;
}

