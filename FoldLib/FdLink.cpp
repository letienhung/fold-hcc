#include "FdLink.h"
#include "RodNode.h"
#include "PatchNode.h"
#include "CustomDrawObjects.h"

FdLink::FdLink( FdNode* n1, FdNode* n2 )
	: Link(n1, n2)
{
	mSeg.P0 = n1->mBox.Center;
	mSeg.P1 = n2->mBox.Center;
}

void FdLink::draw()
{
	LineSegments ls;
	ls.addLine(mSeg.P0, mSeg.P1, Qt::red);
	ls.draw();

	PointSoup ps(10);
	ps.addPoint(mSeg.Center);
	ps.draw();
}
