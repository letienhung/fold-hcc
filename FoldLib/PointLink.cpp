#include "PointLink.h"
#include "RodNode.h"
#include "PatchNode.h"
#include "DistSegSeg.h"

PointLink::PointLink( FdNode* n1, FdNode* n2 )
	:FdLink(n1, n2)
{
	if (n1->mType == FdNode::ROD)
	{
		RodNode* node1 = (RodNode*)n1;
		Geom::Segment rod1 = node1->mRod;
		
		// rod-rod
		if (n2->mType == FdNode::ROD)
		{
			RodNode* node2 = (RodNode*)n2;
			Geom::Segment rod2 = node2->mRod;

			Geom::DistSegSeg dss(rod1, rod2);
			mLink = Geom::Segment(dss.mClosestPoint0, dss.mClosestPoint1);
			mPos = mLink.Center;
		}
		// rod-patch
		else
		{
			PatchNode* node2 = (PatchNode*)n2;
			Geom::Rectangle rect2 = node2->mPatch;
		}
	}
	// patch-rod
	else if (n2->mType == FdNode::ROD)
	{
		PatchNode* node1 = (PatchNode*)n1;
		Geom::Rectangle rect1 = node1->mPatch;

		RodNode* node2 = (RodNode*)n2;
		Geom::Segment rod2 = node2->mRod;
	}
}
