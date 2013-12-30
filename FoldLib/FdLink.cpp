#include "FdLink.h"
#include "RodNode.h"
#include "PatchNode.h"
#include "CustomDrawObjects.h"

#include "FdUtility.h"
#include "DistSegSeg.h"
#include "DistSegRect.h"
#include "DistRectRect.h"
#include "Numeric.h"


FdLink::FdLink( FdNode* n1, FdNode* n2, bool detect /*= true*/ )
	: Link(n1, n2)
{
	if (detect)	detectHinges();
}

FdLink::FdLink( FdLink& other )
	:Link(other)
{
	hinges = other.hinges;
}

FdLink::~FdLink()
{
}

Structure::Link* FdLink::clone()
{
	return new FdLink(*this);
}

void FdLink::draw()
{
}

void FdLink::detectHinges()
{
	hinges.clear();

	FdNode* n1 = (FdNode*)node1;
	FdNode* n2 = (FdNode*)node2;

	if (n1->mType == FdNode::PATCH)
	{
		PatchNode* patch1 = (PatchNode*)n1;

		// patch-patch
		if (n2->mType == FdNode::PATCH)
		{
			PatchNode* patch2 = (PatchNode*)n2;

			if (patch1->mPatch.containsOneEdge(patch2->mPatch))
				detectHinges(patch1, patch2);
			else
				detectHinges(patch2, patch1);
		}
		// patch-rod
		else
		{
			detectHinges((RodNode*)n2, patch1);
		}
	}
	else
	{
		RodNode* rod1 = (RodNode*)n1;

		// rod-patch
		if (n2->mType == FdNode::PATCH)
		{
			detectHinges(rod1, (PatchNode*)n2);
		}
		// rod-rod
		else
		{
			RodNode* rod2 = (RodNode*)n2;

			// to do
		}
	}
}

// branch is on base: hinge is one edge of branch
void FdLink::detectHinges( PatchNode* base, PatchNode* branch )
{
	// hinge axis
	Vector3 baseNormal = base->mPatch.Normal;
	QVector<Geom::Segment> perpEdges = branch->mPatch.getPerpEdges(baseNormal);
	Geom::DistSegRect dsr1(perpEdges[0], base->mPatch);
	Geom::DistSegRect dsr2(perpEdges[1], base->mPatch);
	Geom::Segment hingeSeg = (dsr1.get() < dsr2.get()) ? perpEdges[0] : perpEdges[1];
	Vector3 hingeAxis = hingeSeg.Direction;

	// branch axis
	Vector3 origin = hingeSeg.P0;
	QVector<Geom::Segment> edges = branch->mPatch.getPerpEdges(hingeAxis);
	Geom::Segment branchSeg = edges[0].contains(origin) ? edges[0] : edges[1];
	Vector3 branchAxis = branchSeg.Center - origin;

	// base axis 
	Vector3 crossHingeBranch = cross(hingeAxis, branchAxis);
	Vector3 baseAxis = base->mPatch.getProjectedVector(crossHingeBranch);

	// two hinges
	hinges << Hinge(node1, node2, origin,  branchAxis, baseAxis, hingeAxis, hingeSeg.length());
	hinges << Hinge(node1, node2, origin, -baseAxis, branchAxis, hingeAxis, hingeSeg.length());
}

void FdLink::detectHinges( RodNode* rnode, PatchNode* pnode )
{
	Geom::Segment distSeg = getDistSegment(rnode, pnode);
	Vector3 origin = distSeg.P0;
	double e = rnode->mRod.Extent / 4;

	Vector3 v1 = pnode->mPatch.Axis[0];
	Vector3 v2 = pnode->mPatch.Axis[1];

	Vector3 upV = rnode->mRod.Center - origin;

	// to do
}