#include "DistSegRect.h"
#include "DistLineRect.h"

Geom::DistSegRect::DistSegRect(Segment& seg, Rectangle& rect)
{
	mSegment = &seg;
	mRectangle = &rect;
}

void Geom::DistSegRect::compute()
{
	Line line(mSegment->Center, mSegment->Direction);
	DistLineRect queryLR(line, *mRectangle);
	double sqrDist = queryLR.getSquared();
	mSegmentParameter = queryLR.mLineParameter;

	if (mSegmentParameter >= -mSegment->Extent)
	{
		if (mSegmentParameter <= mSegment->Extent)
		{
			mClosestPoint0 = queryLR.mClosestPoint0;
			mClosestPoint1 = queryLR.mClosestPoint1;
			mRectCoord[0] = queryLR.mRectCoord[0];
			mRectCoord[1] = queryLR.mRectCoord[1];
		}
		else
		{
			//mClosestPoint0 = mSegment->P1;
			//DistPoint3Rectangle3<Real> queryPR(mClosestPoint0,
			//	*mRectangle);
			//sqrDist = queryPR.GetSquared();
			//mClosestPoint1 = queryPR.GetClosestPoint1();
			//mSegmentParameter = mSegment->Extent;
			//mRectCoord[0] = queryPR.GetRectangleCoordinate(0);
			//mRectCoord[1] = queryPR.GetRectangleCoordinate(1);
		}
	}
	else
	{
		//mClosestPoint0 = mSegment->P0;
		//DistPoint3Rectangle3<Real> queryPR(mClosestPoint0, *mRectangle);
		//sqrDist = queryPR.GetSquared();
		//mClosestPoint1 = queryPR.GetClosestPoint1();
		//mSegmentParameter = -mSegment->Extent;
		//mRectCoord[0] = queryPR.GetRectangleCoordinate(0);
		//mRectCoord[1] = queryPR.GetRectangleCoordinate(1);
	}
}
