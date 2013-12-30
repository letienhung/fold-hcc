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
	mLink1 = FdGraph::addLink(part, panel1);

	if (panel2)
	{
		mPanel2 = (PatchNode*)panel2->clone();
		mPanel2->isCtrlPanel = true;
		Graph::addNode(mPanel2);
		mLink2 = FdGraph::addLink(part, panel2);
	}

	// chain length
	if (mPart->mType == FdNode::PATCH)
	{
		PatchNode* part_patch = (PatchNode*)mPart;
		Geom::Rectangle &patch = part_patch->mPatch;
		Vector3 hingeAxis = mLink1->hinges[0].hZ;
		mLength = patch.Extent[patch.getPerpAxisId(hingeAxis)];
	}
	else
	{
		RodNode* part_rod = (RodNode*)mPart;
		mLength = part_rod->mRod.length();
	}
}

