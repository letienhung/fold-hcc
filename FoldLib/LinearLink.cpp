#include "LinearLink.h"
#include "PatchNode.h"
#include "DistRectRect.h"

LinearLink::LinearLink( FdNode* n1, FdNode* n2 )
	: FdLink(n1, n2)
{
	if (n1->mType == FdNode::PATCH && n1->mType == FdNode::PATCH)
	{
		PatchNode* node1 = (PatchNode*)n1;
		Geom::Rectangle rect1 = node1->mPatch;

		PatchNode* node2 = (PatchNode*)n2;
		Geom::Rectangle rect2 = node2->mPatch;

		Geom::DistRectRect drr(rect1, rect2);
		mLink.setFromEnds(drr.mClosestPoint0, drr.mClosestPoint1);
	}	
}
